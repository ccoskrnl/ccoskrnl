// #include <cstdarg>
// #include <cstdint>
// #include <cstdlib>
// #include <ostream>
// #include <new>

#include "../types.h"
// extern "C" {

#define abs(integer)            ((integer) > 0 ? (integer) : -(integer))

/// Arccosine (f64)
///
/// Computes the inverse cosine (arc cosine) of the input value.
/// Arguments must be in the range -1 to 1.
/// Returns values in radians, in the range of 0 to pi.
double acos(double x);

/// Arccosine (f32)
///
/// Computes the inverse cosine (arc cosine) of the input value.
/// Arguments must be in the range -1 to 1.
/// Returns values in radians, in the range of 0 to pi.
float acosf(float x);

/// Inverse hyperbolic cosine (f64)
///
/// Calculates the inverse hyperbolic cosine of `x`.
/// Is defined as `log(x + sqrt(x*x-1))`.
/// `x` must be a number greater than or equal to 1.
double acosh(double x);

/// Inverse hyperbolic cosine (f32)
///
/// Calculates the inverse hyperbolic cosine of `x`.
/// Is defined as `log(x + sqrt(x*x-1))`.
/// `x` must be a number greater than or equal to 1.
float acoshf(float x);

/// Arcsine (f64)
///
/// Computes the inverse sine (arc sine) of the argument `x`.
/// Arguments to asin must be in the range -1 to 1.
/// Returns values in radians, in the range of -pi/2 to pi/2.
double asin(double x);

/// Arcsine (f32)
///
/// Computes the inverse sine (arc sine) of the argument `x`.
/// Arguments to asin must be in the range -1 to 1.
/// Returns values in radians, in the range of -pi/2 to pi/2.
float asinf(float x);

/// Inverse hyperbolic sine (f64)
///
/// Calculates the inverse hyperbolic sine of `x`.
/// Is defined as `sgn(x)*log(|x|+sqrt(x*x+1))`.
double asinh(double x);

/// Inverse hyperbolic sine (f32)
///
/// Calculates the inverse hyperbolic sine of `x`.
/// Is defined as `sgn(x)*log(|x|+sqrt(x*x+1))`.
float asinhf(float x);

/// Arctangent (f64)
///
/// Computes the inverse tangent (arc tangent) of the input value.
/// Returns a value in radians, in the range of -pi/2 to pi/2.
double atan(double x);

/// Arctangent of y/x (f64)
///
/// Computes the inverse tangent (arc tangent) of `y/x`.
/// Produces the correct result even for angles near pi/2 or -pi/2 (that is, when `x` is near 0).
/// Returns a value in radians, in the range of -pi to pi.
double atan2(double y, double x);

/// Arctangent of y/x (f32)
///
/// Computes the inverse tangent (arc tangent) of `y/x`.
/// Produces the correct result even for angles near pi/2 or -pi/2 (that is, when `x` is near 0).
/// Returns a value in radians, in the range of -pi to pi.
float atan2f(float y, float x);

/// Arctangent (f32)
///
/// Computes the inverse tangent (arc tangent) of the input value.
/// Returns a value in radians, in the range of -pi/2 to pi/2.
float atanf(float x);

/// Inverse hyperbolic tangent (f64)
///
/// Calculates the inverse hyperbolic tangent of `x`.
/// Is defined as `log((1+x)/(1-x))/2 = log1p(2x/(1-x))/2`.
double atanh(double x);

/// Inverse hyperbolic tangent (f32)
///
/// Calculates the inverse hyperbolic tangent of `x`.
/// Is defined as `log((1+x)/(1-x))/2 = log1p(2x/(1-x))/2`.
float atanhf(float x);

///
/// Computes the cube root of the argument.
double cbrt(double x);

/// Cube root (f32)
///
/// Computes the cube root of the argument.
float cbrtf(float x);

/// Ceil (f64)
///
/// Finds the nearest integer greater than or equal to `x`.
double ceil(double x);

/// Ceil (f32)
///
/// Finds the nearest integer greater than or equal to `x`.
float ceilf(float x);

/// Sign of Y, magnitude of X (f64)
///
/// Constructs a number with the magnitude (absolute value) of its
/// first argument, `x`, and the sign of its second argument, `y`.
double copysign(double x, double y);

/// Sign of Y, magnitude of X (f32)
///
/// Constructs a number with the magnitude (absolute value) of its
/// first argument, `x`, and the sign of its second argument, `y`.
float copysignf(float x, float y);

double cos(double x);

float cosf(float x);

/// Hyperbolic cosine (f64)
///
/// Computes the hyperbolic cosine of the argument x.
/// Is defined as `(exp(x) + exp(-x))/2`
/// Angles are specified in radians.
double cosh(double x);

/// Hyperbolic cosine (f64)
///
/// Computes the hyperbolic cosine of the argument x.
/// Is defined as `(exp(x) + exp(-x))/2`
/// Angles are specified in radians.
float coshf(float x);

/// Error function (f64)
///
/// Calculates an approximation to the “error function”, which estimates
/// the probability that an observation will fall within x standard
/// deviations of the mean (assuming a normal distribution).
double erf(double x);

/// Complementary error function (f64)
///
/// Calculates the complementary probability.
/// Is `1 - erf(x)`. Is computed directly, so that you can use it to avoid
/// the loss of precision that would result from subtracting
/// large probabilities (on large `x`) from 1.
double erfc(double x);

/// Error function (f32)
///
/// Calculates an approximation to the “error function”, which estimates
/// the probability that an observation will fall within x standard
/// deviations of the mean (assuming a normal distribution).
float erff(float x);

/// Complementary error function (f32)
///
/// Calculates the complementary probability.
/// Is `1 - erf(x)`. Is computed directly, so that you can use it to avoid
/// the loss of precision that would result from subtracting
/// large probabilities (on large `x`) from 1.
float erfcf(float x);

/// Exponential, base *e* (f64)
///
/// Calculate the exponential of `x`, that is, *e* raised to the power `x`
/// (where *e* is the base of the natural system of logarithms, approximately 2.71828).
double exp(double x);

double exp10(double x);

float exp10f(float x);

/// Exponential, base 2 (f64)
///
/// Calculate `2^x`, that is, 2 raised to the power `x`.
double exp2(double x);

/// Exponential, base 2 (f32)
///
/// Calculate `2^x`, that is, 2 raised to the power `x`.
float exp2f(float x);

/// Exponential, base *e* (f32)
///
/// Calculate the exponential of `x`, that is, *e* raised to the power `x`
/// (where *e* is the base of the natural system of logarithms, approximately 2.71828).
float expf(float x);

/// Exponential, base *e*, of x-1 (f64)
///
/// Calculates the exponential of `x` and subtract 1, that is, *e* raised
/// to the power `x` minus 1 (where *e* is the base of the natural
/// system of logarithms, approximately 2.71828).
/// The result is accurate even for small values of `x`,
/// where using `exp(x)-1` would lose many significant digits.
double expm1(double x);

/// Exponential, base *e*, of x-1 (f32)
///
/// Calculates the exponential of `x` and subtract 1, that is, *e* raised
/// to the power `x` minus 1 (where *e* is the base of the natural
/// system of logarithms, approximately 2.71828).
/// The result is accurate even for small values of `x`,
/// where using `exp(x)-1` would lose many significant digits.
float expm1f(float x);

/// Absolute value (magnitude) (f64)
/// Calculates the absolute value (magnitude) of the argument `x`,
/// by direct manipulation of the bit representation of `x`.
double fabs(double x);

/// Absolute value (magnitude) (f32)
/// Calculates the absolute value (magnitude) of the argument `x`,
/// by direct manipulation of the bit representation of `x`.
float fabsf(float x);

/// Positive difference (f64)
///
/// Determines the positive difference between arguments, returning:
/// * x - y	if x > y, or
/// * +0	if x <= y, or
/// * NAN	if either argument is NAN.
///
/// A range error may occur.
double fdim(double x, double y);

/// Positive difference (f32)
///
/// Determines the positive difference between arguments, returning:
/// * x - y	if x > y, or
/// * +0	if x <= y, or
/// * NAN	if either argument is NAN.
///
/// A range error may occur.
float fdimf(float x, float y);

/// Floor (f64)
///
/// Finds the nearest integer less than or equal to `x`.
double floor(double x);

/// Floor (f32)
///
/// Finds the nearest integer less than or equal to `x`.
float floorf(float x);

/// Floating multiply add (f64)
///
/// Computes `(x*y)+z`, rounded as one ternary operation:
/// Computes the value (as if) to infinite precision and rounds once to the result format,
/// according to the rounding mode characterized by the value of FLT_ROUNDS.
double fma(double x, double y, double z);

/// Floating multiply add (f32)
///
/// Computes `(x*y)+z`, rounded as one ternary operation:
/// Computes the value (as if) to infinite precision and rounds once to the result format,
/// according to the rounding mode characterized by the value of FLT_ROUNDS.
float fmaf(float x, float y, float z);

double fmax(double x, double y);

float fmaxf(float x, float y);

double fmin(double x, double y);

float fminf(float x, float y);

double fmod(double x, double y);

float fmodf(float x, float y);

double hypot(double x, double y);

float hypotf(float x, float y);

int32_t ilogb(double x);

int32_t ilogbf(float x);

double j0(double x);

double y0(double x);

float j0f(float x);

float y0f(float x);

double j1(double x);

double y1(double x);

float j1f(float x);

float y1f(float x);

double jn(int32_t n, double x);

double yn(int32_t n, double x);

float jnf(int32_t n, float x);

float ynf(int32_t n, float x);

double ldexp(double x, int32_t n);

float ldexpf(float x, int32_t n);

double lgamma(double x);

float lgammaf(float x);

double log(double x);

double log10(double x);

float log10f(float x);

double log1p(double x);

float log1pf(float x);

double log2(double x);

float log2f(float x);

float logf(float x);

double nextafter(double x, double y);

float nextafterf(float x, float y);

double pow(double x, double y);

float powf(float x, float y);

double remainder(double x, double y);

float remainderf(float x, float y);

double rint(double x);

float rintf(float x);

double round(double x);

float roundf(float x);

double scalbn(double x, int32_t n);

float scalbnf(float x, int32_t n);

double sin(double x);

float sinf(float x);

double sinh(double x);

float sinhf(float x);

double sqrt(double x);

float sqrtf(float x);

double tan(double x);

float tanf(float x);

double tanh(double x);

float tanhf(float x);

double tgamma(double x);

float tgammaf(float x);

double trunc(double x);

float truncf(float x);

// }  // extern "C"
