/**
 * libc_shims.c -- Minimal C library function implementations
 * for Sega Saturn (SGL) where no libc is linked.
 *
 * GCC may emit implicit calls to memcpy/memset/memmove for struct
 * copies and zero-initialization even with -fno-builtin.
 */

#include <stddef.h>
#include <stdint.h>

void *memcpy(void *dest, const void *src, size_t n)
{
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;
    if (((uintptr_t)d & 3) == 0 && ((uintptr_t)s & 3) == 0)
    {
        uint32_t *d32 = (uint32_t *)d;
        const uint32_t *s32 = (const uint32_t *)s;
        while (n >= 4)
        {
            *d32++ = *s32++;
            n -= 4;
        }
        d = (unsigned char *)d32;
        s = (const unsigned char *)s32;
    }
    while (n--)
        *d++ = *s++;
    return dest;
}

void *memmove(void *dest, const void *src, size_t n)
{
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;
    if (d < s)
    {
        while (n--)
            *d++ = *s++;
    }
    else
    {
        d += n;
        s += n;
        while (n--)
            *--d = *--s;
    }
    return dest;
}

void *memset(void *ptr, int value, size_t n)
{
    unsigned char *p = (unsigned char *)ptr;
    unsigned char v = (unsigned char)value;
    if (((uintptr_t)p & 3) == 0 && n >= 4)
    {
        uint32_t v32 = (uint32_t)v | ((uint32_t)v << 8) |
                       ((uint32_t)v << 16) | ((uint32_t)v << 24);
        uint32_t *p32 = (uint32_t *)p;
        while (n >= 4)
        {
            *p32++ = v32;
            n -= 4;
        }
        p = (unsigned char *)p32;
    }
    while (n--)
        *p++ = v;
    return ptr;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
    const unsigned char *p1 = (const unsigned char *)s1;
    const unsigned char *p2 = (const unsigned char *)s2;
    while (n--)
    {
        if (*p1 != *p2)
            return *p1 - *p2;
        p1++;
        p2++;
    }
    return 0;
}

size_t strlen(const char *s)
{
    const char *p = s;
    while (*p) p++;
    return (size_t)(p - s);
}

int strcmp(const char *s1, const char *s2)
{
    while (*s1 && (*s1 == *s2))
    {
        s1++;
        s2++;
    }
    return (unsigned char)*s1 - (unsigned char)*s2;
}

int strncmp(const char *s1, const char *s2, size_t n)
{
    while (n && *s1 && (*s1 == *s2))
    {
        s1++;
        s2++;
        n--;
    }
    if (n == 0) return 0;
    return (unsigned char)*s1 - (unsigned char)*s2;
}

char *strchr(const char *s, int c)
{
    while (*s)
    {
        if (*s == (char)c)
            return (char *)s;
        s++;
    }
    return (c == '\0') ? (char *)s : (void *)0;
}

char *strrchr(const char *s, int c)
{
    const char *last = (void *)0;
    while (*s)
    {
        if (*s == (char)c)
            last = s;
        s++;
    }
    if (c == '\0')
        return (char *)s;
    return (char *)last;
}

char *strncpy(char *dest, const char *src, size_t n)
{
    char *d = dest;
    while (n && *src)
    {
        *d++ = *src++;
        n--;
    }
    while (n--)
        *d++ = '\0';
    return dest;
}

char *strcpy(char *dest, const char *src)
{
    char *d = dest;
    while ((*d++ = *src++))
        ;
    return dest;
}

void abort(void)
{
    while (1) { asm("nop"); }
}

static int __errno_val = 0;
int *__errno(void)
{
    return &__errno_val;
}
