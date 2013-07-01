#include <iostream>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "rda_message.hpp"

namespace archive2 {

std::ostream &
operator << (std::ostream & os, const rda_message & rm)
{
    using std::endl;

    os << "RDA Message, " << rm.payload.length() << " bytes" << endl
       << "  Type: " << rm.message_type << endl
       << "  T/S:  " << rm.timestamp << endl;

    return os;
}

} // namespace archive2
