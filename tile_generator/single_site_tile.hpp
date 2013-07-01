#ifndef RSME_INCLUDED_SINGLE_SITE_TILE_HPP
#define RSME_INCLUDED_SINGLE_SITE_TILE_HPP

#include <map>
#include <utility>
#include <boost/tuple/tuple.hpp>
#include <boost/gil/utilities.hpp>
#include <boost/gil/typedefs.hpp>
#include <boost/gil/pixel.hpp>
#include <boost/gil/color_base_algorithm.hpp>
#include <boost/gil/channel_algorithm.hpp>
#include <boost/shared_ptr.hpp>

#include "geo_math.hpp"
#include "tile_coord.hpp"
#include "sample_cut.hpp"
#include "../base_extract/simple_cut.hpp"

namespace tile_generator {

namespace gil = boost::gil;

void write_green_tile(const base_extract::simple_cut & cut, const long t_x,
        const long t_y, const int t_z, const char * filename);
bool write_colorized_tile(const base_extract::simple_cut & cut, const long t_x,
        const long t_y, const int t_z, const char * filename);

/*
 * Colorized tone mapping operator. The color table is static to the class, and
 * calculated color values are cached in the color table as an optimization.
 */
template <typename PixelType>
struct colorized_tmo
{
    colorized_tmo()
    {
        /*
         * The radar measured values are truncated at the first decimal place
         * to increase cache usage.
         */
        if (!color_table_initialized)
        {
            color_table[-320] = gil::rgba8_pixel_t(0x7a, 0x6c, 0x86, 0x00);
            color_table[   0] = gil::rgba8_pixel_t(0x7a, 0x6c, 0x86, 0x00);
            color_table[ 100] = gil::rgba8_pixel_t(0x7a, 0x6c, 0x86, 0x7f);
            color_table[ 250] = gil::rgba8_pixel_t(0x1a, 0xb7, 0x6a, 0xff);
            color_table[ 350] = gil::rgba8_pixel_t(0x0b, 0x51, 0x0d, 0xff);
            color_table[ 420] = gil::rgba8_pixel_t(0xdf, 0xca, 0x1a, 0xff);
            color_table[ 500] = gil::rgba8_pixel_t(0xb8, 0x08, 0x10, 0xff);
            color_table[ 550] = gil::rgba8_pixel_t(0x85, 0x09, 0x0a, 0xff);
            color_table[ 620] = gil::rgba8_pixel_t(0xcb, 0x1c, 0xe5, 0xff);
            color_table[ 700] = gil::rgba8_pixel_t(0x39, 0x9c, 0xcc, 0xff);
            color_table[ 800] = gil::rgba8_pixel_t(0xff, 0xff, 0xff, 0xff);
            color_table[1000] = gil::rgba8_pixel_t(0xff, 0xff, 0xff, 0xff);
        }
    }

    /*
     * Public interface, returns the color corresponding to the radar value.
     */
    PixelType operator()(const radar_value_t & rv) const
    {
        using boost::get;

        typedef typename gil::kth_semantic_element_reference_type<
            PixelType, 3>::type alpha_chan_ref_t;

        PixelType ret = lookup(rv.first);

        gil::semantic_at_c<3>(ret) =
            static_cast<unsigned char>
            (gil::semantic_at_c<3>(ret) * rv.second);

        return ret;
    }

private:
    /*
     * Static storage of the color table, and a flag indicating its
     * initialization status.
     */
    typedef typename std::map<int, PixelType> color_table_t;
    static color_table_t color_table;
    static bool color_table_initialized;

    /*
     * This is the meat of this TMO, it searches in the color table for a
     * cached value, and returns it. If no cached value is found, it linearly
     * interpolates the value from the nearest neighbors and caches it before
     * returning.
     *
     * NOTE: The caching is currently disabled because certain patterns of
     * access (particularly at high zoom levels) cause visually apparent
     * accumulation of roundoff error, creating artifacts in the tone mapping
     * and visible borders between the tiles. This actually doesn't turn out to
     * cause a large performance hit.
     */
    PixelType lookup(const float z) const
    {
        using boost::tie;

        const int z_hat = static_cast<int>(z * 10.0);
        typedef typename color_table_t::const_iterator ct_iterator;

        ct_iterator ct_iter = color_table.find(z_hat);
        if (ct_iter != color_table.end())
            // Direct hit
            return ct_iter->second;
        else
        {
            // Miss, find the bounding pair and interpolate
            ct_iterator lower_iter, upper_iter;
            tie(lower_iter, upper_iter) = bounding_pair(color_table, z_hat);
            if (lower_iter == color_table.end())
                lower_iter = color_table.begin();
            if (upper_iter == color_table.end())
                --upper_iter;
            
            const int delta_z = upper_iter->first - lower_iter->first;
            const int d_z = z_hat - lower_iter->first;
            const float mu = static_cast<float>(d_z) / delta_z;

            PixelType result;

            for (int k = 0;
                    k < gil::num_channels<PixelType>::value;
                    ++k)
                // FIXME: This typecast isn't generic
                result[k] = static_cast<unsigned char>
                    (lower_iter->second[k] * (1.0 - mu)
                        + upper_iter->second[k] * mu);

            return result;
        }
    }
};

/*
 * Static storage for color table and init flag.
 */
template <typename PixelType>
    typename colorized_tmo<PixelType>::color_table_t
    colorized_tmo<PixelType>::color_table;
template <typename PixelType>
    bool
    colorized_tmo<PixelType>::color_table_initialized(false);

/*
 * Extremely simplistic TMO, puts a scaled measurement value in the green
 * channel, setting all others to zero. Validity is stored in alpha.
 */
template <typename PixelType>
struct green_tmo
{
    PixelType operator()(const radar_value_t & rv) const
    {
        using boost::get;

        typedef typename gil::kth_semantic_element_reference_type<
            PixelType, 3>::type alpha_chan_ref_t;
        typedef typename gil::kth_semantic_element_reference_type<
            PixelType, 1>::type green_chan_ref_t;

        PixelType ret(0);

        // FIXME: These typecasts aren't generic.
        gil::semantic_at_c<1>(ret) = 
            static_cast<unsigned char>
            (((rv.first + 32.0) / 127.0)
                * gil::channel_traits<green_chan_ref_t>::max_value());
            //(rv.first * 2.0 + 66.0);
        gil::semantic_at_c<3>(ret) =
            static_cast<unsigned char>
            (rv.second
                * gil::channel_traits<alpha_chan_ref_t>::max_value());

        return ret;
    }
};

/*
 * Implements a virtual image_view concept that generates a tile at specified
 * tile coordinates from the given simple_cut. Uses the tone mapping operator
 * specified to convert radar measurements to colors.
 *
 * The shared pointer thing is because the constructor in GIL's virtual image
 * view constructor (and evidently a number of other parts of the code where
 * this is used) aggravatingly passes by value instead of const reference.
 */
template <typename PixelType, typename Tmo>
struct sampled_cut
{
    typedef gil::point2<ptrdiff_t> point_t;
    typedef sampled_cut            const_t;
    typedef PixelType              value_type;
    typedef value_type             reference;
    typedef value_type             const_reference;
    typedef point_t                argument_type;
    typedef reference              result_type;
    BOOST_STATIC_CONSTANT(bool, is_mutable=false);

    sampled_cut(const base_extract::simple_cut & the_cut, const long tile_x,
            const long tile_y, const int tile_z)
        : cut(the_cut), t_x(tile_x), t_y(tile_y), t_z(tile_z),
            filter_width_meters(calculate_filter_width(tile_y, tile_z)),
            significance_threshold_met(new bool(false)) { }

    result_type operator()(const point_t & p) const
        { return sample_tone_mapped(p, 0.5, 0.5); }

    bool has_significant_data(void) const
        { return *significance_threshold_met; }

private:
    sampled_cut() { }
    const base_extract::simple_cut & cut;
    const long t_x, t_y;
    const int t_z;
    const float filter_width_meters;
    boost::shared_ptr<bool> significance_threshold_met;
    Tmo tmo;

    result_type sample_tone_mapped(const point_t & p, const float d_x,
            const float d_y) const
    {
        using boost::tie;

        double lat, lon;
        tie(lat, lon) = pixel_mercator_to_latlon(t_x, t_y,
                p.x + d_x, p.y + d_y, t_z);

        const radar_value_t rv =
            sample_gaussian(cut, lat, lon, filter_width_meters);
        PixelType ret = tmo(rv);

        if (gil::semantic_at_c<3>(ret) > 0)
            *significance_threshold_met = true;

        return ret;
    }

    static float calculate_filter_width(const long t_y, const int t_z)
    {
        using boost::get;

        const float delta_lat =
            get<0>(pixel_mercator_to_latlon(0, t_y, 0.0, 0.0, t_z)) -
            get<0>(pixel_mercator_to_latlon(0, t_y, 0.0, 1.0, t_z));
        return MEAN_EARTH_RADIUS * delta_lat;
    }

    result_type oversample_gauss_5pt(const point_t & p) const
    {
        /*
         * This is a 5-point gaussian-weighted oversampler.
         */
        result_type out, center;
        result_type res[4];

        // counter-clockwise from left, rotated counter-clockwise 22.5 degrees.
        res[0] = sample_tone_mapped(p,  1.4239,  0.1173);
        res[1] = sample_tone_mapped(p,  0.1173, -0.4239);
        res[2] = sample_tone_mapped(p, -0.4239,  0.8827);
        res[3] = sample_tone_mapped(p,  0.8827,  1.4239);
        center = sample_tone_mapped(p, 0.5, 0.5);

        for (int k = 0;
                k < gil::num_channels<result_type>::value;
                ++k)
        {
            out[k] =
                static_cast<unsigned char>(
                    res[0][k] * 0.125 +
                    res[1][k] * 0.125 +
                    res[2][k] * 0.125 +
                    res[3][k] * 0.125 +
                    center[k] * 0.500);
        }

        return out;
    }
};

} // namespace tile_generator

#endif // RSME_INCLUDED_SINGLE_SITE_TILE_HPP
