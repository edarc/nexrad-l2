#include <iostream>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "endian.hpp"

#include "binary_util.hpp"
#include "rda_message_segment.hpp"

namespace archive2 {

using boost::integer::ubig32_t;
using boost::integer::ubig16_t;

std::istream &
operator >> (std::istream & is, rda_message_segment & rms)
{
    const size_t FIXED_PAYLOAD_LEN = 2416;
    const size_t M31_HEADER_OFFSET = 4;

    // Length given in halfwords, not bytes
    unsigned long message_len = 2 * read_binary<ubig16_t, unsigned int>(is);

    is.ignore(1); // Drop RDA Redundant Channel

    rms.message_type        = read_binary<uint8_t, unsigned int>(is);
    rms.message_sequence_nr = read_binary<ubig16_t, unsigned int>(is);

    unsigned long mjd, msec;
    mjd  = read_binary<ubig16_t, unsigned long>(is);
    msec = read_binary<ubig32_t, unsigned long>(is);
    rms.timestamp = convert_nexrad_mjd(mjd, msec);

    rms.nr_segments = read_binary<ubig16_t, unsigned int>(is);
    rms.segment_nr  = read_binary<ubig16_t, unsigned int>(is);

    if (rms.message_type == 31u)
        //rms.payload.resize(message_len - M31_HEADER_OFFSET);
        read_string_chunk(is, message_len - M31_HEADER_OFFSET, rms.payload);
    else
    {
        read_string_chunk(is, FIXED_PAYLOAD_LEN, rms.payload);
        rms.payload.resize(message_len);
    }

    return is;
}

std::ostream &
operator << (std::ostream & os, const rda_message_segment & rms)
{
    using std::endl;

    os << "RDA Message Segment, " << rms.payload.length() << " bytes" << endl
       << "  Type: " << rms.message_type << endl
       << "  Seq:  " << rms.message_sequence_nr << endl
       << "  T/S:  " << rms.timestamp << endl
       << "  Segs: " << rms.nr_segments << endl
       << "  SegN: " << rms.segment_nr << endl;

    return os;
}

} // namespace archive2
