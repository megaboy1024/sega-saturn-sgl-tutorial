/**
 * @file saturn_ramcart.h
 * Sega Saturn RAM expansion cartridge detection and initialization.
 *
 * Derived from joengine/jo_engine/core.c (RAM cart support).
 *
 * Cart types:
 *   1 MB (8 Mbit):  two 512 KB zones at 0x22400000 and 0x22600000
 *   4 MB (32 Mbit):  one 2 MB zone at 0x22400000
 *
 * Usage:
 *   ramcart_type_t cart = ramcart_init();
 *   // cart == RAMCART_4MB, RAMCART_1MB, or RAMCART_NONE
 */

#ifndef SATURN_RAMCART_H
#define SATURN_RAMCART_H

#include <stdint.h>

typedef enum {
    RAMCART_NONE,
    RAMCART_1MB,    /* ID 0x5a at 0x24FFFFFF */
    RAMCART_4MB     /* ID 0x5c at 0x24FFFFFF */
} ramcart_type_t;

/* Detect cart type without enabling it */
static inline ramcart_type_t ramcart_detect(void)
{
    volatile uint8_t id = *((volatile uint8_t *)0x24FFFFFF);
    if (id == 0x5a) return RAMCART_1MB;
    if (id == 0x5c) return RAMCART_4MB;
    return RAMCART_NONE;
}

/*
 * Enable cart and configure A-bus.
 * Returns detected cart type (RAMCART_NONE if no cart present).
 */
static inline ramcart_type_t ramcart_init(void)
{
    ramcart_type_t type = ramcart_detect();
    if (type == RAMCART_NONE)
        return RAMCART_NONE;

    /* Enable extended RAM cartridge (SMPC register) */
    *((volatile int *)0x257efffe) = 1;

    /* Configure A-bus CS1 timing */
    *((volatile int *)0x25fe0080) = 0x23301ff0;
    *((volatile int *)0x25fe0033) = 0x00000013;

    return type;
}

/* Memory regions */
#define RAMCART_4MB_BASE   ((void *)0x22400000)
#define RAMCART_4MB_SIZE   0x200000   /* 2 MB */
#define RAMCART_1MB_BASE_A ((void *)0x22400000)
#define RAMCART_1MB_SIZE_A 0x80000    /* 512 KB */
#define RAMCART_1MB_BASE_B ((void *)0x22600000)
#define RAMCART_1MB_SIZE_B 0x80000    /* 512 KB */

#endif /* SATURN_RAMCART_H */
