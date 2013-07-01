#include <iostream>
#include <fstream>
#include <memory>
#include <vector>
#include <deque>
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

    std::deque<tile_t> tiles;
    std::vector<tile_t> new_tiles;
    tiles.push_back(tile_t(0, 0, 1));
    unsigned int generated = 0;

    while (!tiles.empty())
    {
        using boost::tie;
        using boost::lexical_cast;
        using std::string;
        static const char * tile_fmt = "[%|4|:%|4|:%|4|] %|02| %|04| %|04|";

        long t_x, t_y;
        int t_z;
        tie(t_x, t_y, t_z) = tiles.front(); tiles.pop_front();
        bool subdivide = true;
                
        cout
            << format(tile_fmt) % generated % tiles.size()
                % (generated + tiles.size()) % t_z % t_x % t_y
            << std::flush;

        if (t_z < start_zoom)
        {
            cout 
                << " skipped (underzoom)";
        }
        else
        {
            string path = "out/"
                + cut.radar_identifier + "_"
                + lexical_cast<string>(t_z) + "_"
                + lexical_cast<string>(t_x) + "-"
                + lexical_cast<string>(t_y) + ".png";

            subdivide = write_colorized_tile(cut, t_x, t_y, t_z, path.c_str());
            ++generated;

            if (!subdivide)
                cout << " bailing (below threshold)" << std::endl;
        }

        if (subdivide)
        {
            if (t_z >= end_zoom)
                cout << " bailing (max zoom)" << std::endl;
            else
            {
                cout << std::endl;
                new_tiles.clear();
                find_intersecting_tiles(t_x, t_y, t_z, to_rad(cut.latitude),
                        to_rad(cut.longitude), 300000.0, t_z + 1, new_tiles);

                // This removes the first tile from the results because it's
                // the same one we passed in.
                tiles.insert(tiles.end(), new_tiles.begin() + 1,
                        new_tiles.end());
            }
        }
    }

    return 0;
}
