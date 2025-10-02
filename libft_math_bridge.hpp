#ifndef LIBFT_MATH_BRIDGE_HPP
#define LIBFT_MATH_BRIDGE_HPP

// Forward declarations for libft math utilities that conflict with
// template include guards. These declarations allow the game code to
// call into the libft math implementation without redefining vendor
// headers.
double math_fmod(double value, double modulus);
double math_cos(double value);
double ft_sin(double value);
double math_sqrt(double number);
double math_acos(double dot);
double math_fabs(double number);

#endif // LIBFT_MATH_BRIDGE_HPP
