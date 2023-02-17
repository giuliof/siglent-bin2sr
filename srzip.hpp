#ifndef SRZIP_HPP_
#define SRZIP_HPP_

#include <cstddef>
#include <string>
#include <vector>
#include <fstream>

const size_t SAMPLES_LIMIT = 0x280000;

class SiglentAnalogReader
{
  public:

    SiglentAnalogReader(size_t skip, size_t len);

    void open(const std::string& filename);

    std::vector<uint8_t> chunk(size_t chunk_size);

  private:

    std::ifstream f;

    size_t offset;

    const size_t seek;

    const size_t samples;
};

class SiglentDigitalReader
{
  public:

  SiglentDigitalReader(size_t skip, size_t channels, size_t len);

  void open(const std::string& filename);

  std::vector<uint16_t> chunk(size_t chunk_size);

  private:

  std::vector<std::ifstream> fs;

  size_t octets_read;

  const size_t seek;

  const size_t octets;
};

#endif // SRZIP_HPP_