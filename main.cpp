#include <iostream>
#include <vector>
#include <cstring>
#include <string>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <cmath>

#include <zip.h>
#include <argparse/argparse.hpp>
#include <spdlog/spdlog.h>

#include "siglent_bin.hpp"
#include "srzip.hpp"

zip_t* zip_flush(zip_t* zip, std::string filename)
{
  zip_close(zip);
  return zip_open(filename.c_str(), 0, NULL);
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

  zip_t* zip = zip_open(out_path.c_str(), ZIP_CREATE | ZIP_TRUNCATE, NULL);

  std::vector<std::string> analog_labels;
  std::vector<std::string> digital_labels;

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

      if (chunk.size() == 0)
        break;

      std::vector<float> out_chunk;
      out_chunk.reserve(SAMPLES_LIMIT);

      std::transform(chunk.cbegin(), chunk.cend(), std::back_inserter(out_chunk), [&] (uint8_t sample)
      {
        // TODO documentation
        double ret = double(int(sample)-128) * header.analog_scales[channel].get_value() * 10.7 / 256;
        ret -= header.analog_offsets[channel].get_value();
        for (size_t i = 1; i < oversample_factor; i++)
          out_chunk.push_back((float)ret);
        return (float)ret;
      });

      zip_source_t* source = zip_source_buffer(zip, out_chunk.data(), sizeof(out_chunk[0]) * chunk.size(), 0);

      if (source == NULL)
        std::cout << "error creating source: " << zip_strerror(zip) << std::endl;

      std::stringstream ss;
      ss << "analog-1-" << (header.digital_on ? 16 : 0) + active_channel + 1 << "-" << chunk_idx + 1;

      if (zip_file_add(zip, ss.str().c_str(), source, ZIP_FL_ENC_UTF_8) < 0)
        std::cout << "error adding file: " << zip_strerror(zip) << std::endl;

      // Commit
      zip = zip_flush(zip, out_path.c_str());
    }

    analog_labels.push_back(std::to_string(active_channel));

    active_channel++;
    data_offset += header.analog_size;
  }

  if (header.digital_on)
  {
    int digital_channel_no = 0;

    // TODO get number of channels
    for (size_t i = 0; i < 16; i++)
    {
      if (header.digital_ch_on[i])
      {
        digital_labels.push_back(std::to_string(digital_channel_no));
        digital_channel_no++;
      }
    }

    SiglentDigitalReader reader(data_offset, digital_channel_no, header.digital_size / 8);

    reader.open(in_path);

    for (size_t chunk_idx = 0; ; chunk_idx++)
    {
      spdlog::trace("Reading chunk {}", chunk_idx);

      auto chunk = reader.chunk(SAMPLES_LIMIT);

      if (chunk.size() == 0)
        break;

      zip_source_t* source = zip_source_buffer(zip, chunk.data(), sizeof(chunk[0]) *chunk.size(), 0);

      if (source == NULL)
        std::cout << "error creating source: " << zip_strerror(zip) << std::endl;

      std::stringstream ss;
      ss << "logic-1-" << chunk_idx + 1;

      if (zip_file_add(zip, ss.str().c_str(), source, ZIP_FL_ENC_UTF_8) < 0)
        std::cout << "error adding file: " << zip_strerror(zip) << std::endl;
      
      zip = zip_flush(zip, out_path.c_str());
    }
  }

  // build up meatadata file
  {
    std::string str;

    {
      std::stringstream metadata;
      metadata << "[device 1]" << "\n"
      << "samplerate=" << (int)header.digital_sample_rate.get_value() << "\n";

      if (digital_labels.size() > 0)
      {
        metadata << "total probes=" << digital_labels.size() << "\n";
        metadata << "unitsize=2" << "\n";
        metadata << "capturefile=logic-1" << "\n";
      }

      if (analog_labels.size() > 0)
        metadata << "total analog=" << analog_labels.size() << "\n";

      for (size_t i = 1; const auto& label : digital_labels)
        metadata << "probe" << i++ << "=D" << label << "\n";

      for (size_t i = digital_labels.size() + 1; const auto& label : analog_labels)
        metadata << "analog" << i++ << "=A" << label << "\n";

      str = metadata.str();
    }

    zip_source_t* source = zip_source_buffer(zip, str.c_str(), str.length(), 0);

    if (source == NULL)
      std::cout << "error creating source: " << zip_strerror(zip) << "\n";

    if (zip_file_add(zip, "metadata", source, ZIP_FL_OVERWRITE) < 0)
      std::cout << "error adding file: " << zip_strerror(zip) << "\n";
    
    zip = zip_flush(zip, out_path.c_str());
  }


  // build up version file
  {
    std::string version = "2";

    zip_source_t* source = zip_source_buffer(zip, version.c_str(), version.length(), 0);

    if (source == NULL)
      std::cout << "error creating source: " << zip_strerror(zip) << "\n";

    if (zip_file_add(zip, "version", source, ZIP_FL_OVERWRITE) < 0)
      std::cout << "error adding file: " << zip_strerror(zip) << "\n";
    
    zip = zip_flush(zip, out_path.c_str());
  }

  // Close sr zipfile
  zip_close(zip);
}