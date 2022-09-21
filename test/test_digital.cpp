#include "catch.hpp"

#include "../srzip.hpp"
#include "../utils/stream.hpp"

#include <fstream>

#include <iostream>

TEST_CASE("Srzip digital conversion from oscilloscope-like bin", "[srzip-digital]" ) {
  SiglentDigitalReader reader(0, 5, 8);

  reader.open("test-digital-5ch.bin");

  auto chunk = reader.chunk(32);

  REQUIRE(chunk[0] == 0x00);
  REQUIRE(chunk[1] == 0x01);
  REQUIRE(chunk[2] == 0x02);
  REQUIRE(chunk[3] == 0x03);
  REQUIRE(chunk[7] == 0x07);

  chunk = reader.chunk(32);

  REQUIRE(chunk[0] == 0x00);
  REQUIRE(chunk[1] == 0x01);
  REQUIRE(chunk[2] == 0x02);
  REQUIRE(chunk[3] == 0x03);
  REQUIRE(chunk[7] == 0x07);
}

TEST_CASE("Srzip correct number of chunk files from oscilloscope-like bin", "[srzip-digital]" ) {
  SiglentDigitalReader reader(0, 5, 8);

  reader.open("test-digital-5ch.bin");

  size_t chunk_num = 0;

  SECTION("double split")
  {
    while (reader.chunk(32).size()) chunk_num++;
    REQUIRE(chunk_num == 2);
  }

  SECTION("no split")
  {
    while (reader.chunk(64).size()) chunk_num++;
    REQUIRE(chunk_num == 1);
  }

  SECTION("several split")
  {
    while (reader.chunk(8).size()) chunk_num++;
    REQUIRE(chunk_num == 8);
  }
}