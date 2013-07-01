#include <boost/gil/typedefs.hpp>
#include <boost/gil/virtual_locator.hpp>
#include <boost/gil/image_view.hpp>
#include <boost/gil/extension/io/png_io.hpp>

#include "tile_coord.hpp"
#include "single_site_tile.hpp"
#include "../base_extract/simple_cut.hpp"

namespace tile_generator {

namespace gil = boost::gil;

void
write_green_tile(const base_extract::simple_cut & cut, const long t_x,
        const long t_y, const int t_z, const char * filename)
{
    typedef sampled_cut< gil::rgba8_pixel_t,
            green_tmo<gil::rgba8_pixel_t> >         deref_t;
    typedef deref_t::point_t                        point_t;
    typedef gil::virtual_2d_locator<deref_t, false> locator_t;
    typedef gil::image_view<locator_t>              virt_view_t;

    point_t dim(TILE_DIMENSION_PIXELS, TILE_DIMENSION_PIXELS);
    virt_view_t view(dim, locator_t(point_t(0, 0), point_t(1, 1),
                deref_t(cut, t_x, t_y, t_z)));
    gil::png_write_view(filename, view);
}

bool
write_colorized_tile(const base_extract::simple_cut & cut, const long t_x,
        const long t_y, const int t_z, const char * filename)
{
    typedef sampled_cut< gil::rgba8_pixel_t,
            colorized_tmo<gil::rgba8_pixel_t> >     deref_t;
    typedef deref_t::point_t                        point_t;
    typedef gil::virtual_2d_locator<deref_t, false> locator_t;
    typedef gil::image_view<locator_t>              virt_view_t;

    point_t dim(TILE_DIMENSION_PIXELS, TILE_DIMENSION_PIXELS);
    deref_t sampler(cut, t_x, t_y, t_z);
    virt_view_t view(dim, locator_t(point_t(0, 0), point_t(1, 1), sampler));
    //gil::rgba8_image_t buf(view.dimensions());
    //gil::copy_pixels(view, gil::view(buf));
    gil::png_write_view(filename, view);
    return sampler.has_significant_data();
}

} // namespace tile_generator
