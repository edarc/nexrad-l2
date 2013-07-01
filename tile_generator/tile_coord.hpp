#ifndef RSME_INCLUDED_TILE_COORD_HPP
#define RSME_INCLUDED_TILE_COORD_HPP

#include <boost/tuple/tuple.hpp>

namespace tile_generator {

const int TILE_DIMENSION_PIXELS = 256;

typedef boost::tuple<double, double> double_pair_t;
typedef boost::tuple<long, long> long_pair_t;
typedef boost::tuple<long_pair_t, double_pair_t> tile_coord_t;

double_pair_t calculate_false_offset(const double sphere_circumference_pixels);
double_pair_t apply_raster_transform(const double x, const double y,
        const long sphere_circumference_pixels);
double_pair_t inverse_raster_transform(const double x, const double y,
        const long sphere_circumference_pixels);
tile_coord_t latlon_to_pixel_mercator(const double lat_deg,
        const double lon_deg, const int zoom_level);
double_pair_t pixel_mercator_to_latlon(const long t_x, const long t_y,
        const double dt_x, const double dt_y, const int zoom_level);

} // namespace tile_generator

#endif // RSME_INCLUDED_TILE_COORD_HPP
