#include <iostream>
#include <fstream>
#include <list>
#include <iterator>

#include "archive_reader.hpp"
#include "rda_message.hpp"
#include "radial_generic_format.hpp"

int main(int argc, char ** argv)
{
    using std::cout;
    using std::endl;
    using namespace archive2;
    cout.sync_with_stdio(false);

    if (argc != 2)
    {
        std::cerr << "No archive file" << endl;
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
        try
        {
            radial_generic_format rgf(*rm_it);
            cout << rgf;
        }
        catch (rda_message::wrong_type & e)
        {
            cout << *rm_it;
        }
    }

    return 0;
}
