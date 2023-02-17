#include "siglent_bin.hpp"

#include "utils/stream.hpp"

float header_t::unit_value_t::get_value() const
{
  return value;
}

static header_t::unit_value_t from_bin(std::ifstream& stream) {
  header_t::unit_value_t unit;
  unit.value = deserialize<double>(stream);
  unit.magnitude = deserialize<magnitude_t>(stream);
  unit.unit = deserialize<unit_t>(stream);
  return unit;
}

header_t parse(std::ifstream& stream) {
  header_t h;

  for (size_t i = 0; i < MAX_ANALOG_CHANNELS; i++)
    h.analog_ch_on[i] = deserialize<uint32_t>(stream);

  for (size_t i = 0; i < MAX_ANALOG_CHANNELS; i++)
    h.analog_scales[i] = from_bin(stream);

  for (size_t i = 0; i < MAX_ANALOG_CHANNELS; i++)
    h.analog_offsets[i] = from_bin(stream);

  h.digital_on = deserialize<uint32_t>(stream);

  for (size_t i = 0; i < MAX_DIGITAL_PROBES; i++)
    h.digital_ch_on[i] = deserialize<uint32_t>(stream);

  h.time_div = from_bin(stream);

  h.time_delay = from_bin(stream);

  h.analog_size = deserialize<uint32_t>(stream);

  h.analog_sample_rate = from_bin(stream);

  h.digital_size = deserialize<uint32_t>(stream);

  h.digital_sample_rate = from_bin(stream);

  return h;
}

header_t parse_siglent_header_file(const std::string& filename)
{
  std::ifstream f(filename);
  return parse(f);
}
