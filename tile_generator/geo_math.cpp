#include <cmath>
#include <cassert>
#include "geo_math.hpp"

namespace tile_generator {

/*
 * Haversine function
 */
double
haversin(const double theta)
{
    //using std::cos;
    //return 0.5 * (1.0 - cos(theta));
    using std::sin;
    using std::pow;
    return pow(sin(theta / 2.0), 2.0);
}

/*
 * Compute the great circle distance, in meters, between two lat/lon pairs, in
 * radians. This and its related function central_angle() currently use the
 * haversine formula (a spherical approximation of the geoid). They may
 * potentially be replaced with more complex algorithms for solving in geodetic
 * coordinates, but for now this shall suffice.
 */
double
great_circle_distance(const double lat_a, const double lon_a,
        const double lat_b, const double lon_b)
{
    const double c = central_angle(lat_a, lon_a, lat_b, lon_b);
    return MEAN_EARTH_RADIUS * c;
}

/*
 * Compute the central angle, in radians, between two lat/lon pairs, in
 * radians. Uses the haversine formula with a spherical geoid approximation.
 */
double
central_angle(const double lat_a, const double lon_a, const double lat_b,
        const double lon_b)
{
    using std::cos;
    using std::asin;
    using std::sqrt;

    const double delta_lat = lat_b - lat_a;
    const double delta_lon = lon_b - lon_a;

    const double h = haversin(delta_lat) +
        cos(lat_a) * cos(lat_b) * haversin(delta_lon);
    const double c = 2.0 * asin( sqrt(h) );

    return c;
}

/*
 * Compute the initial bearing, **in degrees**, from a starting point to a
 * destination point given as lat/lons in radians.
 */
float
initial_bearing_deg(const double lat_a, const double lon_a,
        const double lat_b, const double lon_b)
{
    using std::sin;
    using std::cos;
    using std::atan2;
    using std::fmod;

    const float delta_lon = lon_b - lon_a;
    const float opposite = sin(delta_lon) * cos(lat_b);
    const float adjacent = 
        cos(lat_a) * sin(lat_b) - sin(lat_a) * cos(lat_b) * cos(delta_lon);

    const float bearing = atan2(opposite, adjacent);
    const float bearing_deg = (bearing >= 0.0 ? to_deg(bearing)
                                              : to_deg(bearing) + 360.0);

    return bearing_deg;
}

/*
 * Compute slant range in meters along an inclined radar beam given a central
 * angle in radians between the point of interest and the radar site itself.
 * 
 * The effect of this function is to find the slant range (that is, the
 * distance from the radar site along the inclined beam itself) which is
 * directly above a point on the earth at the given angular distance from the
 * site.
 */
float
inclined_slant_range(const double central_angle, const double inclination)
{
    using std::cos;
    using std::sin;

    const double phi = central_angle;
    const double theta = inclination;
    /*
    return (MEAN_EARTH_RADIUS * sin(phi)) /
        (cos(theta) * cos(phi) - sin(theta) * sin(phi));
     */
    // Kill 3 trig calls with a nice identity. Thanks Wolfram Alpha!
    return (MEAN_EARTH_RADIUS * sin(phi)) / cos(theta + phi);
}

} // namespace tile_generator
