#ifndef RSME_INCLUDED_BOUNDS_TEST_HPP
#define RSME_INCLUDED_BOUNDS_TEST_HPP

#include <boost/tuple/tuple.hpp>
#include <vector>
#include <memory>

namespace tile_generator {

typedef boost::tuple<long, long, int> tile_t;

bool test_tile_intersection(const long t_x, const long t_y,
        const int zoom_level, const double lat, const double lon,
        const double distance);
std::auto_ptr< std::vector<tile_t> > find_intersecting_tiles(const tile_t tile,
        const double lat, const double lon, const double distance,
        const int max_zoom_level);
void find_intersecting_tiles(const long t_x, const long t_y, const int t_z,
        const double lat, const double lon, const double distance,
        const int max_zoom_level, std::vector<tile_t> & return_tiles);

} // namespace tile_generator

#endif // RSME_INCLUDED_BOUNDS_TEST_HPP
