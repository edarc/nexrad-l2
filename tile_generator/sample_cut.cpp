#include <map>
#include <cmath>
#include <boost/tuple/tuple.hpp>

#include "sample_cut.hpp"
#include "geo_math.hpp"
#include "../base_extract/simple_cut.hpp"

namespace tile_generator {

/*
 * Get the interpreted value of a particular gate from a given radial. The
 * value is a tuple, with the first being the measured value ("Z") and the
 * second being the validity ("V", where 0.0 is invalid and 1.0 is valid).
 */
inline
radar_value_t
gate_val(const simple_radial & rad, int gate_idx)
{
    /*
     * If the gate position is inside the cone of silence or outside the
     * radar's range, return a completely invalid value with the measurement of
     * the first gate or last gate respectively. This makes interpolation work
     * correctly. (As the interpolation falls off the edge of the coverage
     * area, it gradually changes from a valid to an invalid value with the
     * same measurement, instead of the measurement spuriously falling off to
     * zero.)
     */
    float z = 1.0;
    if (gate_idx < 0)
    {
        gate_idx = 0;
        z = 0.0;
    }
    else if (gate_idx > static_cast<int>(rad.gates.size()) - 1)
    {
        gate_idx = rad.gates.size() - 1;
        z = 0.0;
    }
        
    const unsigned char gate = rad.gates[gate_idx];
    if (gate == 0 || gate == 1)
        return radar_value_t(0.0, 0.0);
    else
        return radar_value_t((gate - rad.offset) / rad.scale, z);
}

/*
 * Sample a radial using a 1/sqrt(2) gaussian filter of the specified width at
 * a given central angle from the radar site. The central angle is used instead
 * of the distance because the radials are individually corrected for slant
 * range.
 */
radar_value_t
sample_radial_gaussian(const simple_radial & rad, const double central_angle,
        const float filter_width_meters)
{
    static const float WASHOUT_ALLOWANCE = 2.00; // samples
    using std::ceil;
    using boost::tie;

    const float range =
        inclined_slant_range(central_angle, to_rad(rad.elevation));

    const float filter_scale =
        (filter_width_meters > rad.range_res_meters
            ? filter_width_meters / rad.range_res_meters
            : 1.0);
    const float position =
        (range - rad.start_range_meters) / rad.range_res_meters;
    
    int near_idx = static_cast<int>
        (position - ceil(filter_scale * WASHOUT_ALLOWANCE));
    int far_idx = static_cast<int>
        (position + ceil(filter_scale * WASHOUT_ALLOWANCE));
    if (near_idx < 0) near_idx = 0;
    if (far_idx < 0) far_idx = 0;
    if (far_idx > static_cast<int>(rad.gates.size()))
        far_idx = rad.gates.size();

    if (near_idx > static_cast<int>(rad.gates.size()))
        return gate_val(rad, near_idx);
    else if (far_idx < 0)
        return gate_val(rad, 0);
    else
    {
        float z_accum = 0.0, v_accum = 0.0, coef_accum = 0.0;
        float z, v, coef;
        for (int k = near_idx;
                k != far_idx + 1;
                ++k)
        {
            radar_value_t rv = gate_val(rad, k);
            z = rv.first; v = rv.second;
            coef = gaussian_power((float(k) - position) / filter_scale);
            z_accum += coef * z;
            v_accum += coef * v;
            coef_accum += coef;
        }

        return radar_value_t(z_accum / coef_accum, v_accum / coef_accum);
    }
}

/*
 * Samples the value of the cut at the given lat/lon, in radians. The value is
 * filtered using using a 1/sqrt(2) gaussian filter of the specified width.
 */
radar_value_t
sample_gaussian(const simple_cut & cut, const double lat, const double lon,
        const float filter_width_meters)
{
    static const float ANGULAR_RESOLUTION = 0.5; // degrees
    static const float RANGE_RESOLUTION = 250.0; // meters
    static const float MAX_FILTER_ASPECT = 2.0; // ratio
    static const float MAX_AZIMUTH_FILTER_SCALE = 20.0; // ratio
    // 1/sqrt(2) gaussian needs two samples washout per side
    static const float WASHOUT_ALLOWANCE = 2.00; // samples
    using boost::get;
    using boost::tie;

    const float theta_deg = 
        initial_bearing_deg(to_rad(cut.latitude), to_rad(cut.longitude),
                lat, lon);
    // Calculate angular distance from the radar site
    const float angular_distance =
        central_angle(to_rad(cut.latitude), to_rad(cut.longitude), lat, lon);
    
    // Calculate azimuth filter width indicated by range distance.
    const float calculated_filter_width =
        to_rad(ANGULAR_RESOLUTION) * angular_distance * MEAN_EARTH_RADIUS;
    // Select the wider of azimuth filter widths indicated by range distance,
    // range resolution, and zoom level.
    float effective_filter_width = calculated_filter_width;
    if (effective_filter_width < filter_width_meters)
        effective_filter_width = filter_width_meters;
    if (effective_filter_width < RANGE_RESOLUTION / MAX_FILTER_ASPECT)
        effective_filter_width = RANGE_RESOLUTION / MAX_FILTER_ASPECT;

    // Calculate the azimuth filter scale factor. I'm not sure why this math
    // comes out twice as wide as it should, but it very obviously does, so
    // correct for it.
    const float calculated_az_filter_scale = 
        (effective_filter_width > (angular_distance * MEAN_EARTH_RADIUS)
            ? effective_filter_width / (angular_distance * MEAN_EARTH_RADIUS)
            : 1.0) * 0.5;
    // Prevent singularity at radar site causing rediculous scale values.
    const float az_filter_scale = 
        (calculated_az_filter_scale < MAX_AZIMUTH_FILTER_SCALE
            ? calculated_az_filter_scale
            : MAX_AZIMUTH_FILTER_SCALE);

    // Select the wider of range filter widths indicated by range distance and
    // zoom level.
    float range_filter_width =
        effective_filter_width / MAX_FILTER_ASPECT;
    if (range_filter_width < filter_width_meters)
        range_filter_width = filter_width_meters;

    // Find the azimuth angles of the edges of the filter kernel
    float theta_start = theta_deg - (az_filter_scale * WASHOUT_ALLOWANCE);
    float theta_stop = theta_deg + (az_filter_scale * WASHOUT_ALLOWANCE);
    if (theta_start < 0.0) theta_start += 360.0;
    if (theta_stop >= 360.0) theta_stop -= 360.0;

    // Get a range covering all radials inside the filter kernel
    base_extract::simple_cut::radials_type::const_iterator
        start_iter, stop_iter, iter, END;
    start_iter = bounding_pair(cut.radials, theta_start).first;
    stop_iter = bounding_pair(cut.radials, theta_stop).second;
    END = cut.radials.end();

    if (start_iter == cut.radials.end())
        --start_iter;
    if (stop_iter == cut.radials.end())
        stop_iter = cut.radials.begin();

    float z_accum = 0.0, v_accum = 0.0, coef_accum = 0.0;
    float z, v, coef;
    for (iter = start_iter;
            iter != stop_iter;)
    {
        tie(z, v) = sample_radial_gaussian(iter->second, angular_distance,
                range_filter_width);
        float x = iter->first - theta_deg;
        if (x > 180.0) x -= 360.0;
        if (x < -180.0) x += 360.0;

        coef = gaussian_power(x / az_filter_scale);
        z_accum += coef * z;
        v_accum += coef * v;
        coef_accum += coef;

        ++iter;
        if (iter == cut.radials.end())
            iter = cut.radials.begin();
    }

    return radar_value_t(z_accum / coef_accum, v_accum / coef_accum);
}

} // namespace tile_generator
