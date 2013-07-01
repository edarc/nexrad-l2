#include <cmath>
#include "tile_coord.hpp"
#include "geo_math.hpp"

namespace tile_generator {

using boost::tuples::tie;

/*
 * Calculate the false easting and false northing offsets given the projection
 * sphere's circumference.
 */
double_pair_t
calculate_false_offset(const double sphere_circumference_pixels)
{
    const double false_easting = -1.0 * sphere_circumference_pixels / 2.0;
    const double false_northing = sphere_circumference_pixels / 2.0;

    return double_pair_t(false_easting, false_northing);
}

/*
 * Apply the pixel coordinate system transform. 
 *
 * Un-transformed pixel coordinates from the Mercator projection use an origin
 * at the equator and the prime meridian, with increasing y pointing north.
 * The raster system used in the tiles has the origin at slightly north of
 * +85deg and the international date line, with increasing y pointing south (as
 * one expects in a raster display).
 *
 * This function applies that transformation.
 */
double_pair_t
apply_raster_transform(const double x, const double y,
        const long sphere_circumference_pixels)
{
    double false_easting, false_northing;
    tie(false_easting, false_northing) =
        calculate_false_offset(sphere_circumference_pixels); 

    return double_pair_t(x - false_easting, false_northing - y);
}

/*
 * Apply the inverse pixel coordinate system transform.
 *
 * Transforms tile pixel coordinates into coordinates that can be used in the
 * inverse Mercator projection to get lat/lons. See `apply_raster_transform`.
 */
double_pair_t
inverse_raster_transform(const double x, const double y,
        const long sphere_circumference_pixels)
{
    double false_easting, false_northing;
    tie(false_easting, false_northing) =
        calculate_false_offset(sphere_circumference_pixels); 

    return double_pair_t(x + false_easting, false_northing - y);
}

/*
 * Maps a lat/lon (in decimal degrees) to the Google Maps pixel-mercator
 * projection at the given zoom level. It returns the following structure::
 *
 *   ((t_x, t_y), (dt_x, dt_y))
 *
 * where `t_x` and `t_y` are the tile coordinate at that zoom level, and `dt_x`
 * and `dt_y` are the actual pixel coordinates from the origin of the given
 * tile.
 */
tile_coord_t
latlon_to_pixel_mercator(const double lat_deg, const double lon_deg,
        const int zoom_level)
{
    const int tiles_per_side = 1 << zoom_level;

    const long sphere_circumference_pixels =
        tiles_per_side * TILE_DIMENSION_PIXELS;
    const double sphere_radius_pixels =
        sphere_circumference_pixels / (2.0 * PI);

    const double lat = to_rad(lat_deg);
    const double lon = to_rad(lon_deg);

    // Apply the Mercator projection
    const double r = sphere_radius_pixels;
    const double projected_x = r * lon;
    const double projected_y = (r / 2.0) * std::log(
            (1.0 + std::sin(lat)) / (1.0 - std::sin(lat)) );

    // Apply the raster transform
    double zoomed_x, zoomed_y;
    tie(zoomed_x, zoomed_y) = apply_raster_transform(projected_x, projected_y,
            sphere_circumference_pixels);

    // Get tile locations and offsets
    const long_pair_t tile_location(
            static_cast<long>(zoomed_x / TILE_DIMENSION_PIXELS),
            static_cast<long>(zoomed_y / TILE_DIMENSION_PIXELS));
    const double_pair_t tile_offset(
            std::fmod(zoomed_x, static_cast<double>(TILE_DIMENSION_PIXELS)),
            std::fmod(zoomed_y, static_cast<double>(TILE_DIMENSION_PIXELS)));

    return tile_coord_t(tile_location, tile_offset);
}

/*
 * Maps a tile location/offset into lat/lon. Returns the following structure::
 *
 *   (lat, lon)
 */
double_pair_t
pixel_mercator_to_latlon(const long t_x, const long t_y, const double dt_x,
        const double dt_y, const int zoom_level)
{
    const long tiles_per_side = 1 << zoom_level;

    const long sphere_circumference_pixels =
        tiles_per_side * TILE_DIMENSION_PIXELS;
    const double sphere_radius_pixels =
        sphere_circumference_pixels / (2.0 * PI);

    // Find absolute pixel coordinates
    const double zoomed_x = t_x * TILE_DIMENSION_PIXELS + dt_x;
    const double zoomed_y = t_y * TILE_DIMENSION_PIXELS + dt_y;

    // Inverse raster transform
    double projected_x, projected_y;
    tie(projected_x, projected_y) = inverse_raster_transform(zoomed_x,
            zoomed_y, sphere_circumference_pixels);

    // Inverse Mercator projection
    const double r = sphere_radius_pixels;
    const double lat = 
        (PI / 2.0) - (2.0 * std::atan( std::exp(-1.0 * projected_y / r) ));
    const double lon = projected_x / r;

    return double_pair_t(lat, lon);
}

} // namespace tile_generator
