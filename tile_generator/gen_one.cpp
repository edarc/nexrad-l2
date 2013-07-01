#include <iostream>
#include <fstream>
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

    if (argc != 6)
    {
        cout
            << "usage: gen_one <basefile> <tx> <ty> <zoom> <outfile>"
            << std::endl;
        return 1;
    }

    int t_x, t_y, t_z;
    try
    {
        t_x = boost::lexical_cast<int>(argv[2]);
        t_y = boost::lexical_cast<int>(argv[3]);
        t_z = boost::lexical_cast<int>(argv[4]);
    }
    catch (boost::bad_lexical_cast & e)
    {
        cout << "tx, ty, or tz not an integer" << std::endl;
        return 1;
    }

    simple_cut cut;
    {
        std::ifstream ifs(argv[1], std::ios::binary);
        boost::archive::binary_iarchive ia(ifs);
        ia >> cut;
    }

    if (test_tile_intersection(t_x, t_y, t_z, to_rad(cut.latitude),
                to_rad(cut.longitude), 300000.0))
    {
        write_colorized_tile(cut, t_x, t_y, t_z, argv[5]);
        cout << "200\n";
    }
    else
        cout << "404\n";

    return 0;
}
