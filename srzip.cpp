#include "srzip.hpp"

#include <iostream>

#include <zip.h>

#include <vector>
#include <cstring>
#include <string>
#include <fstream>

SiglentAnalogReader::SiglentAnalogReader(size_t skip, size_t len)
: seek(skip),
samples(len)
{
}

void SiglentAnalogReader::open(const std::string& filename)
{
  f = std::ifstream(filename);
  if (!f.is_open())
    throw std::runtime_error("Failed opening in analog read");

  f.seekg(seek);

  if (f.eof())
    throw std::runtime_error("Failed opening in analog read");

  offset = 0;
}

std::vector<uint8_t> SiglentAnalogReader::chunk(size_t chunk_size)
{
  std::vector<uint8_t> ret(std::min(chunk_size, samples - offset));

  // f not opened

  f.read((char*)ret.data(), ret.size() * sizeof(ret[0]));

  offset += ret.size();

  // Check if eof?

  return ret;
}

SiglentDigitalReader::SiglentDigitalReader(size_t skip, size_t channels, size_t len)
: seek(skip),
octets(len)
{
  for (size_t i = 0; i < channels; i++)
    fs.push_back(std::ifstream());
}

void SiglentDigitalReader::open(const std::string& filename)
{
  for (int i = 0; auto& f : fs)
  {
    f.open(filename);
    
    if (!f.is_open())
      throw std::runtime_error("Failed opening in digital read");
    
    f.seekg(seek + octets * i);

    if (f.eof())
      throw std::runtime_error("Failed reading in digital read");

    i++;
  }

  // Check if eof?
  octets_read = 0;
}

// With this method, a group of samples of up to chunk_size are read.
// These samples will be stored in a single file of .srzip.
// Each octet from siglent bin gives 8 samples in .srzip.
std::vector<uint16_t> SiglentDigitalReader::chunk(size_t chunk_size)
{
  size_t octets_to_be_read = std::min(chunk_size / 8, octets - octets_read);

  std::vector<uint16_t> ret(octets_to_be_read * 8);

  std::fill(ret.begin(), ret.end(), 0);

  // Reads samples in groups of one octect (8 samples)
  for (int channel = 0; auto& f : fs)
  {
    std::vector<uint8_t> ch(octets_to_be_read);

    f.read((char*)ch.data(), ch.size());
    // Check if eof?

    for (size_t base = 0; base < octets_to_be_read; base++)
    {
      for (size_t bit = 0; bit < 8; bit++)
        ret.at(base * 8 + bit) |= ((((uint16_t)ch.at(base) >> bit) & 0x01) << channel);
    }

    channel++;
  }

  octets_read += octets_to_be_read;

  return ret;
}
