#ifndef RSME_INCLUDED_GEO_MATH_HPP
#define RSME_INCLUDED_GEO_MATH_HPP

#include <boost/tuple/tuple.hpp>

namespace tile_generator {

const double PI = 3.14159265358979323846;

/*
 * This value is an approximation to the radius of curvature of the geoid at 39
 * degrees latitude, which is an eyeballed estimate of the center of the US.
 */
const double MEAN_EARTH_RADIUS = 6364784.3;

double haversin(const double theta);
double great_circle_distance(const double lat_a, const double lon_a,
        const double lat_b, const double lon_b);
double central_angle(const double lat_a, const double lon_a,
        const double lat_b, const double lon_b);
float initial_bearing_deg(const double lat_a, const double lon_a,
        const double lat_b, const double lon_b);
float inclined_slant_range(const double central_angle,
        const double inclination);

/*
 * Convert degrees to radians
 */
template <typename ValueType>
ValueType
to_rad(const ValueType v)
{
    return (static_cast<ValueType>(PI) / static_cast<ValueType>(180.0)) * v;
}

/*
 * Convert radians to degrees
 */
template <typename ValueType>
ValueType
to_deg(const ValueType v)
{ 
    return (static_cast<ValueType>(180.0) / static_cast<ValueType>(PI)) * v;
}

} // namespace tile_generator

#endif // RSME_INCLUDED_GEO_MATH_HPP
