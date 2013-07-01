#define EXP_POLY_DEGREE 3

#define POLY0(x, c0) _mm_set1_ps(c0)
#define POLY1(x, c0, c1) \
    _mm_add_ps(_mm_mul_ps(POLY0(x, c1), x), _mm_set1_ps(c0))
#define POLY2(x, c0, c1, c2) \
    _mm_add_ps(_mm_mul_ps(POLY1(x, c1, c2), x), _mm_set1_ps(c0))
#define POLY3(x, c0, c1, c2, c3) \
    _mm_add_ps(_mm_mul_ps(POLY2(x, c1, c2, c3), x), _mm_set1_ps(c0))
#define POLY4(x, c0, c1, c2, c3, c4) \
    _mm_add_ps(_mm_mul_ps(POLY3(x, c1, c2, c3, c4), x), _mm_set1_ps(c0))
#define POLY5(x, c0, c1, c2, c3, c4, c5) \
    _mm_add_ps(_mm_mul_ps(POLY4(x, c1, c2, c3, c4, c5), x), \
            _mm_set1_ps(c0))

__m128 exp2(__m128 x)
{
   __m128i ipart;
   __m128 fpart, expipart, expfpart;

   x = _mm_min_ps(x, _mm_set1_ps( 129.00000f));
   x = _mm_max_ps(x, _mm_set1_ps(-126.99999f));

   // ipart = int(x - 0.5)
   ipart = _mm_cvtps_epi32(_mm_sub_ps(x, _mm_set1_ps(0.5f)));

   // fpart = x - ipart
   fpart = _mm_sub_ps(x, _mm_cvtepi32_ps(ipart));

   // expipart = (float) (1 << ipart)
   expipart = _mm_castsi128_ps(_mm_slli_epi32(
               _mm_add_epi32(ipart, _mm_set1_epi32(127)), 23));

   // minimax polynomial fit of 2**x, in range [-0.5, 0.5[ */
#if EXP_POLY_DEGREE == 5
   expfpart = POLY5(fpart, 9.9999994e-1f, 6.9315308e-1f, 2.4015361e-1f,
           5.5826318e-2f, 8.9893397e-3f, 1.8775767e-3f);
#elif EXP_POLY_DEGREE == 4
   expfpart = POLY4(fpart, 1.0000026f, 6.9300383e-1f, 2.4144275e-1f,
           5.2011464e-2f, 1.3534167e-2f);
#elif EXP_POLY_DEGREE == 3
   expfpart = POLY3(fpart, 9.9992520e-1f, 6.9583356e-1f, 2.2606716e-1f,
           7.8024521e-2f);
#elif EXP_POLY_DEGREE == 2
   expfpart = POLY2(fpart, 1.0017247f, 6.5763628e-1f, 3.3718944e-1f);
#else
#error
#endif

   return _mm_mul_ps(expipart, expfpart);
}


