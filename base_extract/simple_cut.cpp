#include <map>
#include <utility>
#include <algorithm>

#include "../reader/radial_generic_format.hpp"
#include "simple_cut.hpp"

namespace base_extract {

using archive2::radial_generic_format;

simple_radial::simple_radial(const radial_generic_format & rgf,
        const int moment_nr)
{
    azimuth_nr         = rgf.azimuth_nr;
    azimuth            = rgf.azimuth;
    elevation          = rgf.elevation;
    start_range_meters = rgf.moments[moment_nr].start_range * 1000.0;
    range_res_meters   = rgf.moments[moment_nr].range_res * 1000.0;
    scale              = rgf.moments[moment_nr].scale;
    offset             = rgf.moments[moment_nr].offset;

    gates.resize(rgf.moments[moment_nr].gates.size());
    std::copy(rgf.moments[moment_nr].gates.begin(),
            rgf.moments[moment_nr].gates.end(),
            gates.begin());
}

simple_cut::simple_cut(const radial_generic_format & rgf)
    : radials(azimuth_indexer())
{
    radar_identifier = rgf.radar_identifier;
    latitude         = rgf.vol_constants.latitude;
    longitude        = rgf.vol_constants.longitude;
    geo_elevation    = rgf.vol_constants.geo_elevation;
    vcp_nr           = rgf.vol_constants.vcp;
    start_timestamp  = rgf.timestamp;
    end_timestamp    = rgf.timestamp;
}

void
simple_cut::push(const radial_generic_format & rgf, const int moment_nr)
{
    radials.insert(std::make_pair(rgf.azimuth, simple_radial(rgf, moment_nr)));
    if (rgf.timestamp > end_timestamp)
        end_timestamp = rgf.timestamp;
}

void
simple_cut::push(const simple_radial & rad)
{
    radials.insert(std::make_pair(rad.azimuth, rad));
}

} // namespace base_extract
