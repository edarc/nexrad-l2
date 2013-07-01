#include <iostream>
#include <fstream>
#include <list>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "archive_reader.hpp"
#include "rda_message.hpp"
#include "radial_generic_format.hpp"

int main(int argc, char ** argv)
{
    using std::cout;
    using std::endl;
    using namespace archive2;
    cout.sync_with_stdio(false);
    bt::time_facet * fmt = new bt::time_facet("%Y-%m-%d %H:%M:%S");
    cout.imbue(std::locale(cout.getloc(), fmt));

    if (argc != 2)
    {
        std::cerr << "No archive file" << endl;
        std::cout << "error" << endl;
        return 1;
    }

    std::list<rda_message> all_messages;
    {
        std::ifstream ifs(argv[1], std::ios::binary);
        read_archive_messages(ifs, std::back_inserter(all_messages));
    }

    std::list<rda_message>::const_iterator rm_it;
    for (rm_it = all_messages.begin();
            rm_it != all_messages.end();
            ++rm_it)
    {
        if (rm_it->message_type == 31)
        {
            radial_generic_format rgf(*rm_it);
            cout
                << rgf.vol_constants.vcp << endl
                << rgf.timestamp << endl
                << rgf.radar_identifier << endl;
            break;
        }

    }

    return 0;
}
