#include <iostream>
#include <fstream>
#include <memory>
#include <vector>
#include <string>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/tuple/tuple.hpp>

#include "single_site_tile.hpp"
#include "../base_extract/simple_cut.hpp"
#include "bounds_test.hpp"

int main(int argc, char ** argv)
{
    using std::cout;
    using boost::format;
    using namespace base_extract;
    using namespace tile_generator;
    cout.sync_with_stdio(false);

    if (argc != 4)
    {
        cout << "usage: generate <basefile> <startzoom> <endzoom>" << std::endl;
        return 1;
    }

    int start_zoom, end_zoom;
    try
    {
        start_zoom = boost::lexical_cast<int>(argv[2]);
        end_zoom = boost::lexical_cast<int>(argv[3]);
    }
    catch (boost::bad_lexical_cast & e)
    {
        cout << "bad zoomlevel" << std::endl;
        return 1;
    }

    simple_cut cut;
    {
        std::ifstream ifs(argv[1], std::ios::binary);
        boost::archive::binary_iarchive ia(ifs);
        ia >> cut;
    }

    std::auto_ptr< std::vector<tile_t> > tiles_p;
    tiles_p = find_intersecting_tiles(tile_t(0, 0, 1), to_rad(cut.latitude),
            to_rad(cut.longitude), 300000.0, end_zoom);
    const std::vector<tile_t> & tiles(*tiles_p);

    std::vector<tile_t>::const_iterator tile_iter;
    for (tile_iter = tiles.begin();
            tile_iter != tiles.end();
            ++tile_iter)
    {
        using boost::tie;
        using boost::lexical_cast;
        using std::string;

        long t_x, t_y;
        int t_z;
        tie(t_x, t_y, t_z) = *tile_iter;

        if (t_z < start_zoom)
            continue;

        string path = "out/"
            + cut.radar_identifier + "_"
            + lexical_cast<string>(t_z) + "_"
            + lexical_cast<string>(t_x) + "-"
            + lexical_cast<string>(t_y) + ".png";
            
        cout << path << std::endl;
        write_colorized_tile(cut, t_x, t_y, t_z, path.c_str());
    }

    return 0;
}
