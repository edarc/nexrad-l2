#include "bounds_test.hpp"
#include "geo_math.hpp"
#include "tile_coord.hpp"
#include <memory>
#include <vector>
#include <algorithm>
#include <boost/tuple/tuple.hpp>

namespace tile_generator {

/*
 * Test to see if a particular tile intersects a circle of the radius given in
 * meters centered on a site given as lat/lon in radians.
 */
bool
test_tile_intersection(const long t_x, const long t_y, const int zoom_level,
        const double lat, const double lon, const double distance)
{
    using boost::tuples::tie;

    // Find the lat/lon extents of the tile
    const double TDP = static_cast<double>(TILE_DIMENSION_PIXELS);
    double north, east, south, west;
    tie(north, west) = pixel_mercator_to_latlon(
            t_x, t_y, 0.0, 0.0, zoom_level);
    tie(south, east) = pixel_mercator_to_latlon(
            t_x, t_y, TDP, TDP, zoom_level);

    /*
     * The latitude and longitude lines forming the edges of the tile dissect
     * the area into nine sections, like a tic-tac-toe board. The nine return
     * statements below correspond to the numbered cases discussed.
     *
     *  A. Cases 1, 2, 4, and 5 are corner sections. Ex: The northern and
     *     southern extents of the tile are north of the site, and the eastern
     *     and western extents are both east. Test the distance of the
     *     southwest corner.
     *
     *  B. Cases 3, 6, 7, and 8 are side sections. Ex: The northern and
     *     southern extents of the tile are north of the point, but the tile's
     *     east and west extents straddle the point's longitude. Test the
     *     distance of a point with the same longitude of the site, but at the
     *     southern latitude of the tile.
     *
     *  C. Case 9 is the center section. The tile straddles the site in both
     *     latitude and longitude, meaning the site itself is actually on the
     *     tile; return true.
     */

    const bool all_north = north > lat && south > lat;
    const bool all_south = north < lat && south < lat;
    const bool all_east = east > lon && west > lon;
    const bool all_west = east < lon && west < lon;

    if (all_north)
    {
        if (all_east)
            return great_circle_distance(lat, lon, south, west) < distance;
        else if (all_west)
            return great_circle_distance(lat, lon, south, east) < distance;
        else
            return great_circle_distance(lat, lon, south, lon) < distance;
    }
    else if (all_south)
    {
        if (all_east)
            return great_circle_distance(lat, lon, north, west) < distance;
        else if (all_west)
            return great_circle_distance(lat, lon, north, east) < distance;
        else
            return great_circle_distance(lat, lon, north, lon) < distance;
    }
    else
    {
        if (all_east)
            return great_circle_distance(lat, lon, lat, west) < distance;
        else if (all_west)
            return great_circle_distance(lat, lon, lat, east) < distance;
        else
            return true;
    }
}

/*
 * Recursively find all the tiles satisfying test_tile_intersection(), starting
 * at the given tile and searching down to max_zoom_level.
 */
std::auto_ptr< std::vector<tile_t> >
find_intersecting_tiles(const tile_t tile, const double lat, const double lon,
        const double distance, const int max_zoom_level)
{
    using boost::tuples::tie;

    long t_x, t_y;
    int t_z;
    tie(t_x, t_y, t_z) = tile;

    typedef std::vector<tile_t> tile_vec_t;
    std::auto_ptr<tile_vec_t> return_tiles_p(new tile_vec_t);

    find_intersecting_tiles(t_x, t_y, t_z, lat, lon, distance, max_zoom_level,
            *return_tiles_p);

    return return_tiles_p;
}
 
void
find_intersecting_tiles(const long t_x, const long t_y, const int t_z,
        const double lat, const double lon, const double distance,
        const int max_zoom_level, std::vector<tile_t> & return_tiles)
{
    
    if (t_z == max_zoom_level)
    {
        if (test_tile_intersection(t_x, t_y, t_z, lat, lon, distance))
            return_tiles.push_back(tile_t(t_x, t_y, t_z));
    }
    else
    {
        // If this tile doesn't intersect, we don't need to test any tiles
        // beneath it at higher zoom levels.
        if (!test_tile_intersection(t_x, t_y, t_z, lat, lon, distance))
            return;

        return_tiles.push_back(tile_t(t_x, t_y, t_z));

        const long upper_left_t_x = t_x * 2;
        const long upper_left_t_y = t_y * 2;
        const int next_zoom_level = t_z + 1;

        find_intersecting_tiles(upper_left_t_x, upper_left_t_y,
                next_zoom_level, lat, lon, distance, max_zoom_level,
                return_tiles);
        find_intersecting_tiles(upper_left_t_x + 1, upper_left_t_y,
                next_zoom_level, lat, lon, distance, max_zoom_level,
                return_tiles);
        find_intersecting_tiles(upper_left_t_x, upper_left_t_y + 1,
                next_zoom_level, lat, lon, distance, max_zoom_level,
                return_tiles);
        find_intersecting_tiles(upper_left_t_x + 1, upper_left_t_y + 1,
                next_zoom_level, lat, lon, distance, max_zoom_level,
                return_tiles);
    }
}

} // namespace tile_generator
