#ifndef RSME_INCLUDED_ARCHIVE_READER_HPP
#define RSME_INCLUDED_ARCHIVE_READER_HPP

#include <iostream>
#include <list>

#include "archive_primitive.hpp"
#include "compressed_block.hpp"
#include "rda_message.hpp"

namespace archive2 {

template <typename OutputIterator>
void
read_archive_messages(std::istream & is, OutputIterator oi)
{
    std::list<rda_message_segment> all_segments;
    {
        archive_primitive ap;
        is >> ap;
        ap.collect_segments(all_segments);
    }

    while (!all_segments.empty())
    {
        rda_message msg;
        typedef std::list<rda_message_segment>::iterator rms_list_iter;
        std::vector<rms_list_iter> consumed_segment_iters;
        std::vector<rms_list_iter>::const_iterator consumed_segment;

        consumed_segment_iters =
            msg.reassemble(all_segments.begin(), all_segments.end());

        for (consumed_segment = consumed_segment_iters.begin();
                consumed_segment != consumed_segment_iters.end();
                ++consumed_segment)
            all_segments.erase(*consumed_segment);

        *oi = msg;
        ++oi;
    } 
}

} // namespace archive2

#endif // RSME_INCLUDED_ARCHIVE_READER_HPP
