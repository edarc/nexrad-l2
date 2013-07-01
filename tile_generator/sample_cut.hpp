#ifndef RSME_INCLUDED_SAMPLE_CUT_HPP
#define RSME_INCLUDED_SAMPLE_CUT_HPP

#include <cmath>
#include <utility>

#include "geo_math.hpp"
#include "../base_extract/simple_cut.hpp"

namespace tile_generator {

using base_extract::simple_radial;
using base_extract::simple_cut;

typedef std::pair<float, float> radar_value_t;

inline radar_value_t gate_val(const simple_radial & rad, int gate_idx);
radar_value_t sample_radial(const simple_radial & rad,
        const double central_angle);
radar_value_t sample_radial_gaussian(const simple_radial & rad,
        const double central_angle, const float filter_width_meters);
radar_value_t sample(const simple_cut & cut, const double lat,
        const double lon);
radar_value_t sample_gaussian(const simple_cut & cut, const double lat,
        const double lon, const float filter_width_meters);

/*
 * Produce an interpolated value between y1 and y2 using the cosine
 * interpolation function, where mu is the normalized x position between the
 * two values.
 */
template <typename RangeType, typename DomainType>
inline
RangeType
cosine_interpolate(const RangeType y1, const RangeType y2,
        const DomainType mu)
{
    const DomainType mu2 = (1.0 - std::cos(mu * PI)) / 2.0;
    return y1 * (1.0 - mu2) + y2 * mu2;
}

/*
 * Produce an interpolated value between y1 and y2 using a linear
 * interpolation function, where mu is the normalized x position between the
 * two values.
 */
template <typename RangeType, typename DomainType>
inline
RangeType
linear_interpolate(const RangeType y1, const RangeType y2,
        const DomainType mu)
    { return y1 * (1.0 - mu) + y2 * mu; }

template <typename T> inline T gaussian_power(const T x)
    { return gaussian_power_taylor_pw(x); }

template <typename T> inline T gaussian_power_direct(const T x)
    { return std::pow(2.0, -2.0 * x * x); }

/*
 * This calculates the same gaussian power distribution function as the direct
 * version above, but it uses a taylor series expansion to try to beat the very
 * expensive pow() call for efficiency. The expansion was centered at x=0.9,
 * with a degree of 7, which gives good error characteristics from x=0 to about
 * x=1.93. We reflect it around x=0
 */
template<typename T> inline T gaussian_power_taylor(const T x_0)
{
    const T
        // This is the inflection point where the outside slope is minimized.
        X_LIMIT = 1.805, 
        // This is approximately the negative of the value of the polynomial at
        // that inflection point. This should result in a very small positive
        // output beyond X_LIMIT.
        Y_OFFSET = -0.01027;

    // Polynomial coefficients
    const T
        a7 = -0.12434619437619,
        a6 =  0.20930963844945,
        a5 =  0.12556019186025,
        a4 = -0.56595913877614,
        a3 =  0.28293189223611,
        a2 =  0.56186773984557,
        a1 = -0.81181929423979,
        a0 =  0.32533546386048 - Y_OFFSET;

    const T x_abs = std::fabs(x_0);

    // We chop the filter this way so that it won't create a discontinuity,
    // which have a habit of making very ugly image outputs when they are
    // crossed.
    const T x_clip = (x_abs > X_LIMIT) ? X_LIMIT : x_abs;
    const T x = x_clip - 0.9;

    // Estrin's scheme, works well with instruction-level parallelism
    // (pipelining). Hopefully the compiler will eliminate all those common
    // power-of-x subexpressions for us.
    //
    // P7(x) = (C0 +C1x) + (C2 +C3x) x2 + ((C4 +C5x) + (C6 +C7x) x2)x4
    return (a0 + a1 * x) + (a2 + a3 * x) * x * x
        + ((a4 + a5 * x) + (a6 + a7 * x) * x * x) * x * x * x * x;
}

/*
 * This also calculates a power distribution function using Taylor series
 * approximation, except this version uses a two-piece expansion, centered at
 * x=0.5 and x=1.5, to get much better accuracy out to about x=2.23. It's also
 * reflected around x=0, and clips at x=2.23, but this version has much smaller
 * slope at x=0 and x=2.23 to reduce the sharp points and resultant ringing
 * artifacts.
 */
template <typename T> inline T gaussian_power_taylor_pw(const T x_0)
{
    const T
        X_LIMIT = 2.22726,
        X_CROSSOVER = 1.0;

    const T
        X_OFFSET_INNER = 0.5,
        X_OFFSET_OUTER = 1.5;

    // Polynomial coefficients, inner piece
    const T
        a7 =  0.17400738865300,
        a6 =  0.19504045319711,
        a5 = -0.53683952080211,
        a4 = -0.15365608149925,
        a3 =  1.04494768376740,
        a2 = -0.30079497510241,
        a1 = -0.98025814346860,
        a0 =  0.70710678118658,
    // Outer piece
        b7 =  0.01900524221070,
        b6 = -0.09844647924079,
        b5 =  0.09968687365662,
        b4 =  0.06351206060550,
        b3 = -0.27504028874093,
        b2 =  0.32093189823918,
        b1 = -0.18379840190035,
        b0 =  0.04419417382416;

    const T x_abs = std::fabs(x_0);
    const T x_clip = (x_abs > X_LIMIT) ? X_LIMIT : x_abs;
    
    // Estrin's scheme, works well with instruction-level parallelism
    // (pipelining). Hopefully the compiler will eliminate all those common
    // power-of-x subexpressions for us.
    //
    // P7(x) = (C0 +C1x) + (C2 +C3x) x2 + ((C4 +C5x) + (C6 +C7x) x2)x4
    /*
    if (x_clip < X_CROSSOVER)
    {
        const T x = x_clip - X_OFFSET_INNER;
        return (a0 + a1 * x) + (a2 + a3 * x) * x * x
            + ((a4 + a5 * x) + (a6 + a7 * x) * x * x) * x * x * x * x;
    }
    else
    {
        const T x = x_clip - X_OFFSET_OUTER;
        return (b0 + b1 * x) + (b2 + b3 * x) * x * x
            + ((b4 + b5 * x) + (b6 + b7 * x) * x * x) * x * x * x * x;
    }
    */

    // Horner's scheme. This appears to run faster on IA32, even in the absence
    // of fused-multiply-add for floats (?)
    if (x_clip < X_CROSSOVER)
    {
        const T x = x_clip - X_OFFSET_INNER;
        return a0 + x * (a1 + x * (a2 + x * (a3 + x * (a4 + x
            * (a5 + x * (a6 + x * a7))))));
    }
    else
    {
        const T x = x_clip - X_OFFSET_OUTER;
        return b0 + x * (b1 + x * (b2 + x * (b3 + x * (b4 + x
            * (b5 + x * (b6 + x * b7))))));
    }
}

/*
 * Find a pair of iterators that refer to the entries in the container having
 * keys that lie to either side (in sorting order) of the given key value.
 * Requires that container is a model of Sorted Associative Container and
 * Unique Associative Container. Will probably compile but give nonsense
 * results on containers that do not model Unique Associative Container, if
 * there are duplicate keys in the vicinity of the key value of interest.
 */
template <typename SAC> // Sorted Associative Container
std::pair<typename SAC::const_iterator, typename SAC::const_iterator>
bounding_pair(const SAC & container, const typename SAC::key_type & key)
{
    using std::make_pair;

    typename SAC::const_iterator upper_iter, lower_iter;
    
    upper_iter = container.lower_bound(key);
    if (upper_iter == container.begin())
        lower_iter = container.end();
    else
    {
        lower_iter = upper_iter;
        --lower_iter;
    }
    
    return make_pair(lower_iter, upper_iter);
}

} // namespace tile_generator

#endif // RSME_INCLUDED_SAMPLE_CUT_HPP
