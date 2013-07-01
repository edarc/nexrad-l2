#include <iostream>
#include <string>
#include <algorithm>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/bzip2.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

#include "endian.hpp"

#include "binary_util.hpp"
#include "compressed_block.hpp"

namespace archive2 {

namespace bio = boost::iostreams;
using boost::integer::big32_t;

//
// Construct an istream for reading stuff out of the compressed payload.
//
bio::filtering_istream *
bzip2_istringstream(const std::string & payload)
{
    if (payload.length() == 0)
        throw std::istream::failure("Empty payload will hang BZIP2");

    bio::filtering_istream * fs = new bio::filtering_istream;
    fs->push(bio::bzip2_decompressor(), 3500);
    fs->push(bio::array_source(payload.c_str(), payload.length()));
    return fs;
}

//
// Read one compressed_block from the stream.
//
std::istream &
operator >> (std::istream & is, compressed_block & cb)
{
    // The ICD says the control word is the length, but that it is "negative
    // under some circumstances". I'm wildly guessing that should be
    // interpreted as "the absolute value of the control word is the length"
    long control_word = read_binary<big32_t, long>(is);
    unsigned long compressed_length = 
        (control_word > 0) ? static_cast<unsigned long>(control_word)
                           : static_cast<unsigned long>(-control_word);

    std::string payload;
    read_string_chunk(is, compressed_length, payload);
    std::auto_ptr<bio::filtering_istream> fis(bzip2_istringstream(payload));

    // Ignore spurious unspecified header
    fis->ignore(12);

    cb.segments.clear();
    std::istream_iterator<rda_message_segment> eos;
    std::istream_iterator<rda_message_segment> fisi(*fis);
    std::back_insert_iterator< std::list< rda_message_segment > >
        append_iter(cb.segments);
    std::copy(fisi, eos, append_iter);

    {
        using namespace boost::lambda;
        cb.segments.remove_if(
                bind(&rda_message_segment::message_type, _1) == 0u);
    }
    
    return is;
}

//
// Print compressed_block representation.
//
std::ostream &
operator << (std::ostream & os, const compressed_block & cb)
{
    os << "Compressed block: " << cb.segments.size() << " segments"
       << std::endl;

    std::ostream_iterator<rda_message_segment> osi(os);
    std::copy(cb.segments.begin(), cb.segments.end(), osi);

    return os;
}

} // namespace archive2
