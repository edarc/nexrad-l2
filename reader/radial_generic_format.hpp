#ifndef RSME_RADIAL_GENERIC_FORMAT_HPP
#define RSME_RADIAL_GENERIC_FORMAT_HPP

#include <vector>
#include <boost/date_time/posix_time/posix_time_types.hpp>

#include "rda_message.hpp"

namespace archive2 {

namespace bt = boost::posix_time;

struct radial_moment
{
    radial_moment(const std::string & str);

    class invalid_type
        : public std::exception
    {
        const char * what(void) const throw()
        { return "Not a data moment"; }
    };

    std::string  moment_type;
    unsigned int nr_gates;
    float        start_range;
    float        range_res;
    float        scale;
    float        offset;
    std::string  gates;
};

struct volume_constants
{
    volume_constants() { };
    volume_constants(const std::string & str);

    class invalid_type
        : public std::exception
    {
        const char * what(void) const throw()
        { return "Not a volume constant data block"; }
    };

    float        latitude;
    float        longitude;
    int          geo_elevation;
    unsigned int vcp;
};

struct radial_generic_format
    : public rda_message
{
    radial_generic_format(const rda_message & rm);

    static const unsigned int STATUS_START_OF_ELEVATION  = 0x00;
    static const unsigned int STATUS_INTERMEDIATE_RADIAL = 0x01;
    static const unsigned int STATUS_END_OF_ELEVATION    = 0x02;
    static const unsigned int STATUS_START_OF_VOLUME     = 0x03;
    static const unsigned int STATUS_END_OF_VOLUME       = 0x04;

    std::string  radar_identifier;
    unsigned int azimuth_nr;
    float        azimuth;
    unsigned int compression_indicator;
    float        azimuth_res;
    unsigned int radial_status;
    unsigned int elevation_nr;
    unsigned int cut_sector_nr;
    float        elevation;
    float        azimuth_indexing;

    volume_constants vol_constants;
    std::vector<radial_moment> moments;
};

std::istream & operator >> (std::istream & is, radial_generic_format & rgf);
std::ostream & operator << (std::ostream & os,
        const radial_generic_format & rgf);

} // namespace archive2

#endif // RSME_RADIAL_GENERIC_FORMAT_HPP
