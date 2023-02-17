#include "catch.hpp"

#include "../siglent_bin.hpp"
#include "../siglent_data.hpp"

#include <fstream>

TEST_CASE("Testing correct generation of srzip metadata from headers", "[test-data]") {
  header_t header;

  // Default initialiation

  // Turn off all analog channels
  for (auto& ch : header.analog_ch_on)
    ch = false;

  // Turn off all digital channels
  header.digital_on = false;
  for (auto& ch : header.digital_ch_on)
    ch = 0;

  SECTION("only one digital channel")
  {
    header.digital_on = true;
    header.digital_ch_on[5] = true;

    auto analog_labels = getAnalogLabes(header);
    auto digital_labels = getDigitalLabes(header);

    REQUIRE(digital_labels.size() == 1);
    REQUIRE(digital_labels[0] == "6");
    REQUIRE(analog_labels.empty() == true);

    REQUIRE(generateMetadata(header, analog_labels, digital_labels) == 
      "[device 1]\n"
      "samplerate=0\n"
      "total probes=1\n"
      "unitsize=2\n"
      "capturefile=logic-1\n"
      "probe1=D6\n"
      );
  }

  SECTION("only one analog channel")
  {
    header.analog_ch_on[3] = true;

    auto analog_labels = getAnalogLabes(header);
    auto digital_labels = getDigitalLabes(header);

    REQUIRE(analog_labels.size() == 1);
    REQUIRE(analog_labels[0] == "4");
    REQUIRE(digital_labels.empty() == true);

    REQUIRE(generateMetadata(header, analog_labels, digital_labels) ==
      "[device 1]\n"
      "samplerate=0\n"
      "total analog=1\n"
      "analog1=A4\n"
    );
  }

  SECTION("less than 8 digital channels and one analog channel")
  {
    header.analog_ch_on[1] = true;
    header.digital_on = true;
    header.digital_ch_on[1] = true;
    header.digital_ch_on[2] = true;

    auto analog_labels = getAnalogLabes(header);
    auto digital_labels = getDigitalLabes(header);

    REQUIRE(analog_labels.size() == 1);
    REQUIRE(analog_labels[0] == "2");
    REQUIRE(digital_labels.size() == 2);
    REQUIRE(digital_labels[0] == "2");
    REQUIRE(digital_labels[1] == "3");

    REQUIRE(generateMetadata(header, analog_labels, digital_labels) ==
      "[device 1]\n"
      "samplerate=0\n"
      "total probes=2\n"
      "unitsize=2\n"
      "capturefile=logic-1\n"
      "total analog=1\n"
      "probe1=D2\n"
      "probe2=D3\n"
      "analog3=A2\n"
    );

  }

  SECTION("more than 8 digital channels and one analog channel")
  {
    header.analog_ch_on[0] = true;
    header.digital_on = true;
    header.digital_ch_on[0] = true;
    header.digital_ch_on[2] = true;
    header.digital_ch_on[4] = true;
    header.digital_ch_on[6] = true;
    header.digital_ch_on[8] = true;
    header.digital_ch_on[10] = true;
    header.digital_ch_on[12] = true;
    header.digital_ch_on[14] = true;
    header.digital_ch_on[15] = true;

    auto analog_labels = getAnalogLabes(header);
    auto digital_labels = getDigitalLabes(header);

    REQUIRE(analog_labels.size() == 1);
    REQUIRE(analog_labels[0] == "1");
    REQUIRE(digital_labels.size() == 9);
    REQUIRE(digital_labels[0] == "1");

    REQUIRE(generateMetadata(header, analog_labels, digital_labels) == 
      "[device 1]\n"
      "samplerate=0\n"
      "total probes=9\n"
      "unitsize=2\n"
      "capturefile=logic-1\n"
      "total analog=1\n"
      "probe1=D1\n"
      "probe2=D3\n"
      "probe3=D5\n"
      "probe4=D7\n"
      "probe5=D9\n"
      "probe6=D11\n"
      "probe7=D13\n"
      "probe8=D15\n"
      "probe9=D16\n"
      "analog10=A1\n"
    );
  }

  SECTION("no channels")
  {
    auto analog_labels = getAnalogLabes(header);
    auto digital_labels = getDigitalLabes(header);

    REQUIRE(analog_labels.empty() == true);
    REQUIRE(digital_labels.empty() == true);

    REQUIRE(generateMetadata(header, analog_labels, digital_labels) ==
      "[device 1]\n"
      "samplerate=0\n"
    );
  }
}
