#ifndef RSME_INCLUDED_SIMPLE_CUT_HPP
#define RSME_INCLUDED_SIMPLE_CUT_HPP

#include <string>
#include <map>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/date_time/posix_time/time_serialize.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>

#include "../reader/radial_generic_format.hpp"
#include "indexed_map.hpp"

namespace base_extract {

namespace bt = boost::posix_time;
using archive2::radial_generic_format;

struct simple_radial
{
    simple_radial() { };
    simple_radial(const radial_generic_format & rgf, const int moment_nr = 0);

    template <typename Archive> void serialize(Archive & ar,
            const unsigned int version);

    unsigned int azimuth_nr;
    float        azimuth;
    float        elevation;
    float        start_range_meters;
    float        range_res_meters;
    float        scale;
    float        offset;

    std::vector<unsigned char> gates;
};

template <typename Archive>
void
simple_radial::serialize(Archive & ar, const unsigned int version)
{
    ar & azimuth_nr;
    ar & azimuth;
    ar & elevation;
    ar & start_range_meters;
    ar & range_res_meters;
    ar & scale;
    ar & offset;
    ar & gates;
}

struct azimuth_indexer
{
    size_t operator()(const float theta) const
    { return static_cast<size_t>(theta * 5.0); }
};

struct simple_cut
{
    simple_cut() : radials(azimuth_indexer()) { };
    simple_cut(const radial_generic_format & rgf);
    void push(const radial_generic_format & rgf, const int moment_nr = 0);
    void push(const simple_radial & rad);

    template <typename Archive> void serialize(Archive & ar,
            const unsigned int version);

    std::string  radar_identifier;
    float        latitude;
    float        longitude;
    float        geo_elevation;
    unsigned int vcp_nr;
    bt::ptime    start_timestamp;
    bt::ptime    end_timestamp;

    //typedef std::map<float, simple_radial> radials_type;
    //std::map<float, simple_radial> radials;
    typedef indexed_map<float, simple_radial, azimuth_indexer> radials_type;
    indexed_map<float, simple_radial, azimuth_indexer> radials;
};

template <typename Archive>
void
simple_cut::serialize(Archive & ar, const unsigned int version)
{
    ar & radar_identifier;
    ar & latitude;
    ar & longitude;
    ar & geo_elevation;
    ar & vcp_nr;
    ar & start_timestamp;
    ar & end_timestamp;
    ar & radials;
}

} // namespace base_extract

#endif // RSME_INCLUDED_SIMPLE_CUT_HPP
