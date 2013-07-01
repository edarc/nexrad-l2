#ifndef RSME_INCLUDED_COMPRESSED_BLOCK_HPP
#define RSME_INCLUDED_COMPRESSED_BLOCK_HPP

#include <iostream>
#include <list>
#include <algorithm>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

#include "rda_message_segment.hpp"

namespace archive2 {

struct compressed_block
{
    std::list<rda_message_segment> segments;

    template <typename Collection>
    void collect_segments(Collection & col) const;
};

std::istream & operator >> (std::istream & is, compressed_block & cb);
std::ostream & operator << (std::ostream & os, const compressed_block & cb);

template <typename Collection>
void
compressed_block::collect_segments(Collection & col) const
{
    using namespace boost::lambda;
    std::for_each(segments.begin(), segments.end(),
            bind(&Collection::push_back, &col, _1));
}

} // namespace archive2

#endif // RSME_INCLUDED_COMPRESSED_BLOCK_HPP
