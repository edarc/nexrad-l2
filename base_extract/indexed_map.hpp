#ifndef RSME_INCLUDED_ARRAY_MAP_HPP
#define RSME_INCLUDED_ARRAY_MAP_HPP

#include <list>
#include <vector>
#include <utility>

#include <boost/serialization/list.hpp>
#include <boost/serialization/vector.hpp>

namespace base_extract {

using std::pair;
using std::list;
using std::vector;

/*
 * This collection class maintains a linked list of key-value pairs with an
 * index computed by a functor. The index stores iterators into the list,
 * allowing extremely fast search by using a key to compute a subscript into
 * the index to obtain a direct iterator to the nearest item with an equal or
 * greater index value.
 *
 *   1 A 
 *   2 A 
 *   3 A  A
 *   4 B 
 *   5 B  B
 *   6 C 
 *   7 C  C
 *   8 e 
 *   9 e 
 */
template <typename Key, typename T, typename IndexFunctor>
struct indexed_map
{
    typedef Key                                         key_type;
    typedef T                                           data_type;
    typedef pair<Key, T>                                value_type;
    typedef typename vector<value_type>::size_type        size_type;
    typedef typename vector<value_type>::iterator         iterator;
    typedef typename vector<value_type>::const_iterator   const_iterator;
    typedef typename vector<value_type>::reverse_iterator reverse_iterator;
    typedef typename vector<value_type>::const_reverse_iterator
        const_reverse_iterator;
    typedef IndexFunctor                                index_functor_type;

    typedef iterator                                    index_node_type;

    iterator begin(void) { return store.begin(); }
    iterator end(void) { return store.end(); }
    const_iterator begin(void) const { return store.begin(); }
    const_iterator end(void) const { return store.end(); }
    iterator rbegin(void) { return store.rbegin(); }
    iterator rend(void) { return store.rend(); }
    const_iterator rbegin(void) const { return store.rbegin(); }
    const_iterator rend(void) const { return store.rend(); }
    size_type size(void) const { return store.size(); }
    bool empty(void) const { return store.empty(); }

    indexed_map(const index_functor_type & idx) : indexer(idx) { }

    class index_collision : public std::exception
    {
        const char * what(void) const throw()
        { return "Index collision in indexed_map"; }
    };

    iterator insert(const value_type & x)
    {
        size_t idx = indexer(x.first);
        iterator insert_iter;

        if (idx + 1 > index.size())
        {
            // If the index is larger than the largest indexed value we hold,
            // we're inserting the new tail. We insert in front of the end.
            insert_iter = store.end();
        }
        else
        {
            // Otherwise we need to find an iterator for the item that comes
            // after the one we're inserting to pass to store.insert(). The
            // current index slot for this item should give us that iterator.
            iterator next_iter = index[idx];

            // If the item this index slot points to indexes to the same slot,
            // we're colliding. Whoops.
            if (indexer(next_iter->first) == idx)
                throw index_collision();
            else
                insert_iter = next_iter;
        }

        // Insert the item
        iterator inserted_iter = store.insert(insert_iter, x);

        // Now we update the index slots from this new value up to the one that
        // comes after us; that is, the one we inserted in front of.
        /* 
         * This can only work with iterators that don't invalidate. Vector!
        size_t update_from, update_to;
        update_from = idx;
        update_to = (insert_iter == store.end()
                ? index.size()
                : indexer(insert_iter->first));

        for (size_t i = update_from;
                i != update_to;
                ++i)
            index[i] = inserted_iter;
         */
        rebuild_index();

        // We're done
        return inserted_iter;
    }

    /*
     * The reason for the existence of this class: find the first item with a
     * key not less than k. It does this with one computed subscript.
     */
    const_iterator lower_bound(const key_type & k) const
    {
        size_t idx = indexer(k);

        if (idx + 1 > index.size())
            return store.end();
        else
            return index[idx];
    }

    template <typename Archive>
    void
    save(Archive & ar, const unsigned int version) const
    {
        ar & store;
    }

    template <typename Archive>
    void
    load(Archive & ar, const unsigned int version)
    {
        ar & store;
        rebuild_index();
    }

    void
    rebuild_index(void)
    {
        typename vector<value_type>::iterator iter;
        size_t start = 0, stop = 0, i;

        iter = store.end();
        --iter;
        index.resize(indexer(iter->first) + 1);

        for (iter = store.begin();
                iter != store.end();
                ++iter)
        {
            stop = indexer(iter->first) + 1;

            for (i = start; i != stop; ++i)
                index[i] = iter;

            start = stop;
        }

        for (i = start; i != index.size(); ++i)
            index[i] = iter;
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()

private:
    vector<index_node_type> index;
    vector<value_type>      store;
    index_functor_type      indexer;
    
};

} // namespace base_extract

#endif // RSME_INCLUDED_ARRAY_MAP_HPP
