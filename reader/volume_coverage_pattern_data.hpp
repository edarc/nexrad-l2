#ifndef RSME_VOLUME_COVERAGE_PATTERN_DATA_HPP
#define RSME_VOLUME_COVERAGE_PATTERN_DATA_HPP

#include "rda_message.hpp"

namespace archive2 {

struct volume_coverage_pattern_data
    : public rda_message
{
    volume_coverage_pattern_data(const rda_message & rm);

};

} // namespace archive2

#endif // RSME_VOLUME_COVERAGE_PATTERN_DATA_HPP
