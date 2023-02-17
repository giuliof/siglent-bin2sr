#include "siglent_data.hpp"

#include "siglent_bin.hpp"

#include <sstream>

std::vector<std::string> getAnalogLabes(const header_t& header)
{
  std::vector<std::string> labels;

  for (size_t ch = 0; ch < header.analog_ch_on.size(); ch++)
    if (header.analog_ch_on[ch])
      labels.push_back(std::to_string(ch + 1));

  return labels;
}

std::vector<std::string> getDigitalLabes(const header_t& header)
{
  std::vector<std::string> labels;

  if (!header.digital_on)
    return {};

  for (size_t ch = 0; ch < header.digital_ch_on.size(); ch++)
    if (header.digital_ch_on[ch])
      labels.push_back(std::to_string(ch + 1));

  return labels;
}

std::string generateMetadata(const header_t& header, const std::vector<std::string>& analog_labels, const std::vector<std::string>& digital_labels)
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

  return metadata.str();
}
