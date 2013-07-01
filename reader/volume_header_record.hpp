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

    class bad_version
      : public std::exception
    {
    public:
        std::string message;

        bad_version(const std::string & bad)
          : message("Bad VHR version: " + bad)
        { }
        ~bad_version() throw() { }

        virtual const char * what(void) const throw()
        { return message.c_str(); }
    };

    class bad_magic
      : public std::exception
    {
        virtual const char * what(void) const throw()
        { return "Bad VHR magic string."; }
    };
};

std::istream & operator >> (std::istream & is, volume_header_record & vhr);
std::ostream & operator << (std::ostream & os,
        const volume_header_record & vhr);

} // namespace archive2

#endif // RSME_INCLUDED_VOLUME_HEADER_RECORD_HPP
