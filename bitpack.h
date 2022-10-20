#ifndef _BITPACK_H
#define _BITPACK_H

#include <lua.h>
#include <lauxlib.h>

#define BITPACK_RV_SUCCESS 1
#define BITPACK_RV_ERROR   0

#define BITPACK_DEFAULT_MEM_SIZE 32

/** The various bitpack error types. */
typedef enum {
    BITPACK_ERR_NONE          = 0,
    BITPACK_ERR_MALLOC_FAILED = 1,
    BITPACK_ERR_INVALID_INDEX = 2,
    BITPACK_ERR_VALUE_WRONG   = 3,
    BITPACK_ERR_RANGE_TOO_BIG = 4,
    BITPACK_ERR_EMPTY         = 6
} bitpack_err_t;

struct _bitpack_t
{
    unsigned long size;                    /** size of bitpack in bits */
    unsigned long data_size;               /** amount of allocated memory */
    unsigned char *data;                   /** pointer to the acutal data */
};

typedef struct _bitpack_t * bitpack_t;

#define bitpack_init_default() \
        bitpack_init(BITPACK_DEFAULT_MEM_SIZE)

bitpack_t bitpack_init(unsigned long num_bytes);

bitpack_t bitpack_init_from_bytes(const char *bytes, unsigned long num_bytes);

void bitpack_destroy(bitpack_t bp);

unsigned long bitpack_size(bitpack_t bp);

unsigned long bitpack_data_size(bitpack_t bp);

int bitpack_on(bitpack_t bp, unsigned long index);

int bitpack_off(bitpack_t bp, unsigned long index);

int bitpack_get(bitpack_t bp, unsigned long index, unsigned char *bit);

int bitpack_set_bytes(bitpack_t bp, const char *value,
        unsigned long num_bytes, unsigned long index);

int bitpack_get_bytes(bitpack_t bp, unsigned long num_bytes,
        unsigned long index, luaL_Buffer *b);

#define bitpack_append_bytes(bp, value, num_bytes) \
        bitpack_set_bytes(bp, value, num_bytes, bitpack_size(bp))

unsigned long bitpack_to_bytes(bitpack_t bp, luaL_Buffer *b);

#endif
