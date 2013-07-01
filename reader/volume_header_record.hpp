#ifndef RSME_INCLUDED_VOLUME_HEADER_RECORD_HPP
#define RSME_INCLUDED_VOLUME_HEADER_RECORD_HPP

#include <boost/date_time/posix_time/posix_time_types.hpp>

namespace archive2 {

namespace bt = boost::posix_time;

struct volume_header_record
{
    unsigned int version;
    unsigned int extension_nr;
    bt::ptime    volume_recorded;
    std::string  icao_identifier;
};

std::istream & operator >> (std::istream & is, volume_header_record & vhr);
std::ostream & operator << (std::ostream & os,
        const volume_header_record & vhr);

} // namespace archive2

#endif // RSME_INCLUDED_VOLUME_HEADER_RECORD_HPP
