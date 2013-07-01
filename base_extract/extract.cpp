#include <iostream>
#include <fstream>
#include <list>
#include <iterator>
#include <boost/format.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include "../reader/archive_reader.hpp"
#include "../reader/rda_message.hpp"
#include "../reader/radial_generic_format.hpp"
#include "simple_cut.hpp"

template <typename Archive, typename Type>
void
save_to(Archive & ar, const Type & t)
{
    ar << t;
}

int main(int argc, char ** argv)
{
    using std::cout;
    using std::cin;
    using boost::format;
    using namespace archive2;
    using namespace base_extract;
    cout.sync_with_stdio(false);

    std::list<rda_message> all_messages;
    read_archive_messages(cin, std::back_inserter(all_messages));
    simple_cut cut;

    std::list<rda_message>::const_iterator msg_iter;
    for (msg_iter = all_messages.begin();
            msg_iter != all_messages.end();
            ++msg_iter)
    {
        try
        {
            const radial_generic_format radial(*msg_iter);

            if (radial.radial_status ==
                    radial_generic_format::STATUS_START_OF_VOLUME)
                cut = simple_cut(radial);

            if (radial.radial_status ==
                    radial_generic_format::STATUS_START_OF_ELEVATION)
                break;

            cut.push(radial);
        }
        catch (rda_message::wrong_type & e)
            { }
    }

    const std::string filename = cut.radar_identifier + ".base";
    std::ofstream ofs(filename.c_str(), std::ios::binary);
    boost::archive::binary_oarchive oa(ofs);
    save_to(oa, cut);
    
    return 0;
}
