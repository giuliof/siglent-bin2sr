#include <algorithm>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <zip.h>
#include <argparse/argparse.hpp>
#include <spdlog/spdlog.h>

#include "siglent_bin.hpp"
#include "siglent_data.hpp"
#include "srzip.hpp"

int zip_source_save(zip_source_t* src, const char* archive) {
  void* data;
  size_t size;

  if (zip_source_is_deleted(src)) {
    /* new archive is empty, thus no data */
    data = NULL;
  } else {
    zip_stat_t zst;

    if (zip_source_stat(src, &zst) < 0) {
      fprintf(stderr, "can't stat source: %s\n",
              zip_error_strerror(zip_source_error(src)));
      return 1;
    }

    size = zst.size;

    if (zip_source_open(src) < 0) {
      fprintf(stderr, "can't open source: %s\n",
              zip_error_strerror(zip_source_error(src)));
      return 1;
    }
    if ((data = malloc(size)) == NULL) {
      fprintf(stderr, "malloc failed: %s\n", strerror(errno));
      zip_source_close(src);
      return 1;
    }
    if ((zip_uint64_t)zip_source_read(src, data, size) < size) {
      fprintf(stderr, "can't read data from source: %s\n",
              zip_error_strerror(zip_source_error(src)));
      zip_source_close(src);
      free(data);
      return 1;
    }
    zip_source_close(src);
  }

  /* example implementation that writes data to file */
  FILE* fp;

  if (data == NULL) {
    if (remove(archive) < 0 && errno != ENOENT) {
      fprintf(stderr, "can't remove %s: %s\n", archive, strerror(errno));
      return -1;
    }
    return 0;
  }

  if ((fp = fopen(archive, "wb")) == NULL) {
    fprintf(stderr, "can't open %s: %s\n", archive, strerror(errno));
    return -1;
  }
  if (fwrite(data, 1, size, fp) < size) {
    fprintf(stderr, "can't write %s: %s\n", archive, strerror(errno));
    fclose(fp);
    return -1;
  }
  if (fclose(fp) < 0) {
    fprintf(stderr, "can't write %s: %s\n", archive, strerror(errno));
    return -1;
  }

  return 0;
}

zip_t* zip_flush(zip_t* zip, zip_source_t* src) {
  zip_close(zip);

  zip_error_t error;

  zip_error_init(&error);

  if ((zip = zip_open_from_source(src, 0, &error)) == NULL) {
    zip_source_free(src);
  } else {
    zip_source_keep(src);
  }

  zip_error_fini(&error);

  return zip;
}

int main(int argc, const char** argv) {
  // Initialize argument parsing
  argparse::ArgumentParser program("siglent-bin2sr");

  program.add_argument("input").help("Input filename");
  program.add_argument("-o", "--output").help("Output folder");
  program.add_argument("-v", "--verbose").help("Increase verbosity")
    .default_value(false)
    .implicit_value(true);

  try {
    program.parse_args(argc, argv);
  } catch (const std::runtime_error& e) {
    spdlog::error(e.what());
    std::exit(1);
  }

  // Prepare input and output file
  // Default output folder is same as input
  std::filesystem::path in_path = program.get("input");
  std::filesystem::path out_path = in_path.parent_path();

  if (!std::filesystem::exists(in_path)) {
    spdlog::error("Input file {} does not exist", in_path.c_str());
    std::exit(1);
  }

  if (auto fn = program.present("-o")) {
    out_path = *fn;
    if (!std::filesystem::exists(out_path) ||
        !std::filesystem::is_directory(out_path)) {
      spdlog::error("Output folder {} does not exist or is invalid", out_path.c_str());
      std::exit(1);
    }
  }
  out_path /= in_path.stem();
  out_path += ".srzip";

  if (program["--verbose"] == true) {
    spdlog::set_level(spdlog::level::trace);
  }

  // Parse header, else error
  header_t header = parse_siglent_header_file(in_path);

  // Debugging informations
  spdlog::info("Parsed header");
  spdlog::trace("Active analog channels: {}", std::count(header.analog_ch_on.begin(), header.analog_ch_on.end(), true) );
  spdlog::trace("Analog sample rate: {}", header.analog_sample_rate.get_value());
  spdlog::trace("Analog size: {}", header.analog_size);
  if (header.digital_on) {
    spdlog::trace("Active digital channels: {}", std::count(header.digital_ch_on.begin(), header.digital_ch_on.end(), true));
    spdlog::trace("Digital sample rate: {}", header.digital_sample_rate.get_value());
    spdlog::trace("Digital size: {}", header.digital_size);
  }

  zip_t* zip;
  zip_source_t* src;
  zip_error_t error;

  zip_error_init(&error);
  /* create source from buffer */
  if ((src = zip_source_buffer_create(NULL, 0, 1, &error)) == NULL) {
    fprintf(stderr, "can't create source: %s\n", zip_error_strerror(&error));
    zip_error_fini(&error);
    return 1;
  }

  /* open zip archive from source */
  if ((zip = zip_open_from_source(src, ZIP_TRUNCATE, &error)) == NULL) {
    fprintf(stderr, "can't open zip from source: %s\n",
            zip_error_strerror(&error));
    zip_source_free(src);
    zip_error_fini(&error);
    return 1;
  }
  zip_error_fini(&error);

  /* we'll want to read the data back after zip_close */
  zip_source_keep(src);

  const std::vector<std::string> analog_labels =
    getAnalogLabes(header);
  const std::vector<std::string> digital_labels =
    getDigitalLabes(header);

  size_t data_offset = DATA_OFFSET;
  for (size_t channel = 0, active_channel = 0; channel < header.analog_ch_on.size(); channel++)
  {
    if (!header.analog_ch_on[channel])
      continue;

    spdlog::info("Reading analog channel {}", channel);

    SiglentAnalogReader reader(data_offset, header.analog_size);

    reader.open(in_path);

    // If digital enabled, analog may be oversampled
    size_t oversample_factor = 1;
    if (header.digital_on)
      oversample_factor = header.digital_size / header.analog_size;

    for (size_t chunk_idx = 0; ; chunk_idx++)
    {
      spdlog::trace("Reading chunk {}", chunk_idx);

      auto chunk = reader.chunk(SAMPLES_LIMIT / oversample_factor);

      spdlog::trace("Reading chunk - transform {}", chunk_idx);

      if (chunk.size() == 0)
        break;

      std::vector<float> out_chunk;
      out_chunk.reserve(SAMPLES_LIMIT);

      std::transform(chunk.cbegin(), chunk.cend(),
                     std::back_inserter(out_chunk), [&](uint8_t sample) {
                       // TODO documentation
                       double ret = double(int(sample) - 128) *
                                    header.analog_scales[channel].get_value() *
                                    10.7 / 256;
                       ret -= header.analog_offsets[channel].get_value();
                       for (size_t i = 1; i < oversample_factor; i++)
                         out_chunk.push_back((float)ret);
                       return (float)ret;
                     });

      spdlog::trace("Reading chunk - zipping {}", chunk_idx);

      zip_source_t* source = zip_source_buffer(zip, out_chunk.data(), sizeof(out_chunk[0]) * out_chunk.size(), 0);

      if (source == NULL)
        std::cout << "error creating source: " << zip_strerror(zip)
                  << std::endl;

      std::stringstream ss;
      ss << "analog-1-" << (header.digital_on ? digital_labels.size() : 0) + active_channel + 1 << "-" << chunk_idx + 1;

      if (zip_file_add(zip, ss.str().c_str(), source, ZIP_FL_ENC_UTF_8) < 0)
        std::cout << "error adding file: " << zip_strerror(zip) << std::endl;

      spdlog::trace("Reading chunk - flushing {}", chunk_idx);
      
      // Commit
      zip = zip_flush(zip, src);
    }

    active_channel++;
    data_offset += header.analog_size;
  }

  if (header.digital_on)
  {
    int digital_channel_no = digital_labels.size();

    SiglentDigitalReader reader(data_offset, digital_channel_no,
                                header.digital_size / 8);

    reader.open(in_path);

    for (size_t chunk_idx = 0; ; chunk_idx++)
    {
      spdlog::trace("Reading chunk {}", chunk_idx);

      auto chunk = reader.chunk(SAMPLES_LIMIT);

      if (chunk.size() == 0)
        break;

      zip_source_t* source = zip_source_buffer(zip, chunk.data(), sizeof(chunk[0]) * chunk.size(), 0);

      if (source == NULL)
        std::cout << "error creating source: " << zip_strerror(zip)
                  << std::endl;

      std::stringstream ss;
      ss << "logic-1-" << chunk_idx + 1;

      if (zip_file_add(zip, ss.str().c_str(), source, ZIP_FL_ENC_UTF_8) < 0)
        std::cout << "error adding file: " << zip_strerror(zip) << std::endl;

      zip = zip_flush(zip, src);
    }
  }

  {
    std::string str = generateMetadata(header, analog_labels, digital_labels);

    zip_source_t* source = zip_source_buffer(zip, str.c_str(), str.length(), 0);

    if (source == NULL)
      std::cout << "error creating source: " << zip_strerror(zip) << "\n";

    if (zip_file_add(zip, "metadata", source, ZIP_FL_OVERWRITE) < 0)
      std::cout << "error adding file: " << zip_strerror(zip) << "\n";

    zip = zip_flush(zip, src);
  }

  // build up version file
  {
    std::string version = "2";

    zip_source_t* source =
        zip_source_buffer(zip, version.c_str(), version.length(), 0);

    if (source == NULL)
      std::cout << "error creating source: " << zip_strerror(zip) << "\n";

    if (zip_file_add(zip, "version", source, ZIP_FL_OVERWRITE) < 0)
      std::cout << "error adding file: " << zip_strerror(zip) << "\n";

    zip = zip_flush(zip, src);
  }

  // Close sr zipfile
  zip_close(zip);

  // Write back to file
  zip_source_save(src, out_path.c_str());
}
