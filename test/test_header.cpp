#include "catch.hpp"

#include "../siglent_bin.hpp"
#include "../utils/stream.hpp"

#include <fstream>

TEST_CASE("Oscilloscope header deserializer Test", "[bin-deserializer]" ) {
  std::ifstream f("SDS00001.bin");

  header_t header = parse(f);

  REQUIRE(header.analog_ch_on[0] == true);
  REQUIRE(header.analog_ch_on[1] == true);
  REQUIRE(header.analog_ch_on[2] == true);
  REQUIRE(header.analog_ch_on[3] == true);

  REQUIRE(header.digital_on == false);

  REQUIRE(header.analog_scales[0].value == 0.2);
  REQUIRE(header.analog_scales[0].magnitude == magnitude_t::IU);
  REQUIRE(header.analog_scales[0].unit == unit_t::V);

  REQUIRE(header.analog_scales[1].value == 1.0);
  REQUIRE(header.analog_scales[1].magnitude == magnitude_t::IU);
  REQUIRE(header.analog_scales[1].unit == unit_t::V);

  REQUIRE(header.analog_scales[2].value == 1.0);
  REQUIRE(header.analog_scales[2].magnitude == magnitude_t::IU);
  REQUIRE(header.analog_scales[2].unit == unit_t::V);

  REQUIRE(header.analog_scales[3].value == 0.2);
  REQUIRE(header.analog_scales[3].magnitude == magnitude_t::IU);
  REQUIRE(header.analog_scales[3].unit == unit_t::V);

  REQUIRE(header.analog_size == 0.0);

  REQUIRE(header.analog_sample_rate.value == 1000000000.0);
  REQUIRE(header.analog_sample_rate.magnitude == magnitude_t::IU);
}