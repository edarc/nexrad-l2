#ifndef RSME_INCLUDED_SPLAY_MAP_HPP
#define RSME_INCLUDED_SPLAY_MAP_HPP

#include <boost/intrusive/splay_set.hpp>
#include <utility>

namespace base_extract {

using boost::intrusive;

template <typename Key, typename T>
struct splay_map_node
    : public intrusive::splay_set_base_hook<>,
      public std::pair<Key, T>
{
    friend bool operator<(const splay_map_node<Key, T> & a,
            const splay_map_node<Key, T> & b)
        { return a.first < b.first; }
    friend bool operator==(const splay_map_node<Key, T> & a,
            const splay_map_node<Key, T> & b)
        { return a.first == b.first; }
};

template <typename Key, typename T>
struct splay_map
    : public intrusive::splay_set< splay_map_node<Key, T>,
        intrusive::compare< std::greater< splay_map_node<Key, T> > > >
{
    // TODO: adapt member functions
};

} // namespace base_extract

#endif // RSME_INCLUDED_SPLAY_MAP_HPP
