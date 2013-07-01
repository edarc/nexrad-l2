#ifndef RSME_INCLUDED_ARCHIVE_PRIMITIVE_HPP
#define RSME_INCLUDED_ARCHIVE_PRIMITIVE_HPP

#include <string>
#include <list>
#include <iostream>
#include <algorithm>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/ref.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

#include "volume_header_record.hpp"
#include "rda_message_segment.hpp"
#include "compressed_block.hpp"

namespace archive2 {

namespace bt = boost::posix_time;

struct archive_primitive
{
    volume_header_record        header;
    std::list<compressed_block> blocks;

    template <typename Collection>
    void collect_segments(Collection & col) const;
};

std::istream & operator >> (std::istream & is, archive_primitive & ap);
std::ostream & operator << (std::ostream & os, const archive_primitive & ap);

template <typename Collection>
void
archive_primitive::collect_segments(Collection & col) const
{
    using namespace boost::lambda;
    using boost::ref;

    std::for_each(blocks.begin(), blocks.end(),
            bind(&compressed_block::collect_segments<Collection>, _1, ref(col)));
}

} // namespace archive2

#endif // RSME_INCLUDED_ARCHIVE_PRIMITIVE_HPP
