#include <iostream>
#include <string>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "endian.hpp"

#include "binary_util.hpp"
#include "volume_header_record.hpp"

namespace archive2 {

using boost::integer::ubig32_t;
using boost::lexical_cast;

//
// Read a volume_header_record from an input stream
// 
std::istream &
operator >> (std::istream & is, volume_header_record & vhr)
{
    const size_t MAGIC_LEN           = 6;
    const size_t VERSION_LEN         = 2;
    const size_t EXTENSION_NR_LEN    = 3;
    const size_t ICAO_IDENTIFIER_LEN = 4;

    // Read the magic string.
    std::string magic = read_string_chunk(is, MAGIC_LEN);
    if (magic != "AR2V00")
    {
        is.seekg(-MAGIC_LEN, std::ios_base::cur);
        throw volume_header_record::bad_magic();
    }
    
    // Read record version
    std::string version = read_string_chunk(is, VERSION_LEN);
    try
        { vhr.version = lexical_cast<unsigned int>(version); }
    catch (boost::bad_lexical_cast & e)
    {
        is.seekg(-(MAGIC_LEN + VERSION_LEN), std::ios_base::cur);
        throw volume_header_record::bad_version(version);
    }

    is.ignore(1); // Drop period

    // Read extension number
    std::string extension_nr = read_string_chunk(is, EXTENSION_NR_LEN);
    vhr.extension_nr = lexical_cast<unsigned int>(extension_nr);
    
    // Read and convert VCP timestamp
    unsigned long nexrad_mjd, msec_since_midnight;
    nexrad_mjd          = read_binary<ubig32_t, unsigned long>(is);
    msec_since_midnight = read_binary<ubig32_t, unsigned long>(is);
    vhr.volume_recorded = convert_nexrad_mjd(nexrad_mjd, msec_since_midnight);

    // Read radar ID
    vhr.icao_identifier = read_string_chunk(is, ICAO_IDENTIFIER_LEN);

    return is;
}

//
// Print volume_header_record to output stream in readable way.
//
std::ostream &
operator << (std::ostream & os, const volume_header_record & vhr)
{
    using std::endl;

    os << "Version:      " << vhr.version << endl
       << "Extension Nr: " << vhr.extension_nr << endl
       << "Recorded:     " << vhr.volume_recorded << endl
       << "Site ID:      " << vhr.icao_identifier << endl;

    return os;
}

} // namespace archive2
