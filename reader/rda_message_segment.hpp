#ifndef RSME_RDA_MESSAGE_SEGMENT_HPP
#define RSME_RDA_MESSAGE_SEGMENT_HPP

#include <string>
#include <boost/date_time/posix_time/posix_time_types.hpp>

namespace archive2 {

namespace bt = boost::posix_time;

struct rda_message_segment
{
    unsigned int message_type;
    unsigned int message_sequence_nr;
    bt::ptime    timestamp;
    unsigned int nr_segments;
    unsigned int segment_nr;
    std::string  payload;
};

std::istream & operator >> (std::istream & is, rda_message_segment & rms);
std::ostream & operator << (std::ostream & os,
    const rda_message_segment & rms);

} // namespace archive2

#endif // RSME_RDA_MESSAGE_SEGMENT_HPP
