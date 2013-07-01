#ifndef RSME_INCLUDED_UTIL_HPP
#define RSME_INCLUDED_UTIL_HPP

#include <iostream>
#include <string>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/date_time/gregorian/gregorian_types.hpp>

#include "endian.hpp"

namespace archive2 {

namespace bd = boost::gregorian;
namespace bt = boost::posix_time;
using boost::integer::ubig32_t;

void read_string_chunk(std::istream & is, size_t len, std::string & out);
std::string read_string_chunk(std::istream & is, size_t len);

//
// Convert the NEXRAD modified Julian date plus milliseconds format into a
// boost posix time.
//
template <typename Td, typename Tt >
bt::ptime
convert_nexrad_mjd(Td mjd, Tt msec)
{
    using namespace bd;
    const date nexrad_epoch(1970, Jan, 1);

    bd::date volume_date(nexrad_epoch + days(mjd - 1));
    return bt::ptime(volume_date) + bt::millisec(msec);
}

//
// Read a word from a stream using a reinterpret_cast.
//
template < typename ReadType, typename ReturnType >
ReturnType
read_binary(std::istream & is)
{
    ReadType read_value(0);
    is.read(reinterpret_cast<char *>(&read_value), sizeof(read_value));
    return static_cast<ReturnType>(read_value);
}

template <> float read_binary<ubig32_t, float>(std::istream & is);

} // namespace archive2

#endif // RSME_INCLUDED_UTIL_HPP
