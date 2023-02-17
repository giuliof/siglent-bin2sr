#ifndef SIGLENT_DATA_HPP_
#define SIGLENT_DATA_HPP_

#include <vector>
#include <string>

struct header_t;

std::vector<std::string> getAnalogLabes(const header_t& header);
std::vector<std::string> getDigitalLabes(const header_t& header);
std::string generateMetadata(const header_t& header, const std::vector<std::string>& analog_labels, const std::vector<std::string>& digital_labels);

#endif