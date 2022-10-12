#ifndef SIGLENT_BIN_HPP_
#define SIGLENT_BIN_HPP_

#include <inttypes.h>
#include <fstream>
#include <array>

const int MAX_DIGITAL_PROBES = 16;
const int MAX_ANALOG_CHANNELS = 4;
const int DATA_OFFSET = 0x800;

enum class magnitude_t : uint32_t {
  YOCTO,
  ZEPTO,
  ATTO,
  FEMTO,
  PICO,
  NANO,
  MICRO,
  MILLI,
  IU,
  KILO,
  MEGA,
  GIGA,
  TERA,
  PETA
};

enum class unit_t : uint32_t {
  V,
  A,
  VV,
  AA,
  OU,
  W,
  SQRT_V,
  SQRT_A,
  INTEGRAL_V,
  INTEGRAL_A,
  DT_V,
  DT_A,
  DT_DIV,
  HZ,
  S,
  SA,
  PTS,
  NUL,
  DB,
  DBV,
  DBA,
  VPP,
  VDC,
  DBM
};

struct header_t {
  struct unit_value_t {
    double value;
    magnitude_t magnitude;
    unit_t unit;

    float get_value();
  };

  // on/off status of CH1-CH4, 1 - ON, 0 - OFF
  std::array<bool, MAX_ANALOG_CHANNELS>  analog_ch_on;

  // V/div value of CH1-CH4, such as 2.48 mV/div.
  // Unit of value, such as V from 0x1c-0x1f.
  // Units of value’s magnitude (MICRO) from 0x18-0x1b.
  // 64-bit float point, data of value from 0x10-0x17.
  std::array<unit_value_t, MAX_ANALOG_CHANNELS> analog_scales;

  std::array<unit_value_t, MAX_ANALOG_CHANNELS> analog_offsets;

  bool digital_on;

  std::array<uint32_t, MAX_DIGITAL_PROBES> digital_ch_on;

  unit_value_t time_div;

  unit_value_t time_delay;

  uint32_t analog_size;

  unit_value_t analog_sample_rate;

  uint32_t digital_size;

  unit_value_t digital_sample_rate;
};

header_t parse(std::ifstream& stream);

header_t parse_siglent_header_file(const std::string& filename);

#endif  // SIGLENT_BIN_HPP_
