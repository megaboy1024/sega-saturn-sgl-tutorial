/**
 * saturn_limits.h -- Minimal limits.h for SH-2 (32-bit, big-endian)
 * Replaces the broken newlib limits.h that has a missing #include_next target.
 */
#ifndef SATURN_LIMITS_H
#define SATURN_LIMITS_H

#define CHAR_BIT    8

#define SCHAR_MIN   (-128)
#define SCHAR_MAX   127
#define UCHAR_MAX   255

#define CHAR_MIN    SCHAR_MIN
#define CHAR_MAX    SCHAR_MAX

#define SHRT_MIN    (-32768)
#define SHRT_MAX    32767
#define USHRT_MAX   65535

#define INT_MIN     (-2147483647 - 1)
#define INT_MAX     2147483647
#define UINT_MAX    4294967295U

#define LONG_MIN    (-2147483647L - 1)
#define LONG_MAX    2147483647L
#define ULONG_MAX   4294967295UL

#define LLONG_MIN   (-9223372036854775807LL - 1)
#define LLONG_MAX   9223372036854775807LL
#define ULLONG_MAX  18446744073709551615ULL

#ifndef SIZE_MAX
#define SIZE_MAX    UINT_MAX
#endif

#define MB_LEN_MAX  1

#endif /* SATURN_LIMITS_H */
