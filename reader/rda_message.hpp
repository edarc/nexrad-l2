#ifndef RSME_RDA_MESSAGE_HPP
#define RSME_RDA_MESSAGE_HPP

#include <exception>
#include <string>
#include <vector>
#include <boost/date_time/posix_time/posix_time_types.hpp>

namespace archive2 {

namespace bt = boost::posix_time;

class bad_message_segment
  : public std::exception
{
    virtual const char * what(void) const throw()
    { return "Corrupt RDA message segment"; }
};

class reassembly_error
  : public std::exception
{
    virtual const char * what(void) const throw()
    { return "Failed to reassemble message (probably because of missing "
        "segments)"; }
};

struct rda_message
{
    unsigned int message_type;
    bt::ptime    timestamp;
    std::string  payload;

    class wrong_type
      : public std::exception
    {
        virtual const char * what(void) const throw()
        { return "Wrong message type"; }
    };

    template <typename Iterator>
    std::vector<Iterator> reassemble(Iterator begin, Iterator end);
};

std::ostream & operator << (std::ostream & os, const rda_message & rms);

//
// Reassemble a complete message from its segments.
//
// The begin iterator is taken to point at the first segment of the message,
// and the end iterator is a sanity check taken to point at the last of all
// segments.
//
// Returns a vector of iterators indicating which segments in the source
// sequence were used (presumably so they may be deleted from further
// consideration in reassembling other messages).
//
template <typename Iterator>
std::vector<Iterator>
rda_message::reassemble(Iterator begin, Iterator end)
{
    Iterator seg_it = begin;
    std::vector<Iterator> used_segments;
    std::vector<std::string> payloads;

    // Load first segment and figure out what next.
    unsigned int sequence_nr = seg_it->message_sequence_nr;
    unsigned int nr_segments = seg_it->nr_segments;
    message_type = seg_it->message_type;
    timestamp    = seg_it->timestamp;
    payload.clear();

    if (nr_segments == 1)
    {
        // That's all, unfragmented message
        payload = seg_it->payload;
        used_segments.push_back(seg_it);
        return used_segments;
    }
    else
    {
        // Reassemble payloads
        payloads.resize(nr_segments);

        // Find and insert each segment payload in it's place in a sequence, in
        // case they are out of order. Then reassemble them at the end.
        for (; seg_it != end;
                ++seg_it)
        {
            if (seg_it->segment_nr < 1 || seg_it->segment_nr > nr_segments)
                // The segment number is bogus
                throw bad_message_segment();

            if (seg_it->message_sequence_nr != sequence_nr)
                // We've found a segment belonging to another message, skip it
                continue;

            // Place the payload in it's correct slot
            payloads[seg_it->segment_nr - 1] = seg_it->payload;
            used_segments.push_back(seg_it);

            // If we have all the segments, we're done. This does not check for
            // and can be fooled by duplicate segment numbers :(
            if (used_segments.size() == nr_segments)
                break;
        }

        if (used_segments.size() != nr_segments)
            // We didn't find enough segments
            throw reassembly_error();

        // Reassemble the segment payloads into the message payload.
        std::vector<std::string>::const_iterator payload_it;
        for (payload_it = payloads.begin();
                payload_it != payloads.end();
                ++payload_it)
            payload.append(*payload_it);

        return used_segments;
    }
}

} // namespace archive2

#endif // RSME_RDA_MESSAGE_HPP
