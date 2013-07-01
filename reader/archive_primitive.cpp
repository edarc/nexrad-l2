#include <iostream>
#include <algorithm>

#include "endian.hpp"

#include "archive_primitive.hpp"
#include "compressed_block.hpp"

namespace archive2 {

//
// Read an archive_primitive from a stream.
//
std::istream &
operator >> (std::istream & is, archive_primitive & ap)
{
    is.exceptions(std::istream::eofbit | std::istream::badbit |
            std::istream::failbit);
    is >> ap.header;

    std::istream_iterator<compressed_block> eos;
    std::istream_iterator<compressed_block> isi(is);
    std::back_insert_iterator< std::list<compressed_block> >
        append_iter(ap.blocks);
    try
        { std::copy(isi, eos, append_iter); }
    catch (std::istream::failure e)
        { }

    return is;
}

//
// Print an archive_primitive representation.
//
std::ostream &
operator << (std::ostream & os, const archive_primitive & ap)
{
    os << ap.header;

    std::ostream_iterator<compressed_block> osi(os);
    std::copy(ap.blocks.begin(), ap.blocks.end(), osi);

    return os;
}

} // namespace archive2
