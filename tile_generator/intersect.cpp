#include <iostream>
#include <fstream>
#include <memory>
#include <vector>
#include <boost/format.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/tuple/tuple_io.hpp>

#include "../base_extract/simple_cut.hpp"
#include "bounds_test.hpp"
#include "geo_math.hpp"

int main(int argc, char ** argv)
{
    using std::cout;
    using std::cin;
    using boost::format;
    using namespace base_extract;
    using namespace tile_generator;
    cout.sync_with_stdio(false);

    simple_cut cut;
    {
        std::ifstream ifs("KLVX.base", std::ios::binary);
        boost::archive::binary_iarchive ia(ifs);
        ia >> cut;
    }

    std::auto_ptr< std::vector<tile_t> > tiles_p;
    tiles_p = find_intersecting_tiles(tile_t(0, 0, 1), to_rad(cut.latitude),
            to_rad(cut.longitude), 300000.0, 10);
    const std::vector<tile_t> & tiles(*tiles_p);
    
    std::vector<tile_t>::const_iterator tile_iter;
    for (tile_iter = tiles.begin();
            tile_iter != tiles.end();
            ++tile_iter)
        cout << *tile_iter << '\n';

    return 0;
}
