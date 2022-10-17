#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bitpack.h"

#ifdef DEBUG
#define print(format,...) printf(format, ##__VA_ARGS__)
#else
#define print(format,...)
#endif

/* round up to the nearest multiple of 8 */
static unsigned long
round8(unsigned long v) {
    if (v % 8 != 0) {
        v += 8 - (v % 8);
    }
    return v;
}

/* increase the size of a bitpack object, allocating more memory if necessary */
static int
_bitpack_resize(bitpack_t bp, unsigned long new_size) {
    unsigned long new_data_size = round8(new_size) / 8;
    if (new_data_size > bp->data_size) {
        bp->data = realloc(bp->data, new_data_size);
        if (bp->data == NULL) {
            print("memory allocation failed\n");
            return BITPACK_RV_ERROR;
        }
        memset(bp->data + bp->data_size, 0, new_data_size - bp->data_size);
        bp->data_size = new_data_size;
    }
    bp->size = new_size;
    return BITPACK_RV_SUCCESS;
}

bitpack_t
bitpack_init(unsigned long num_bytes) {
    bitpack_t      bp;
    unsigned char *data;

    bp = malloc(sizeof(struct _bitpack_t));
    if (bp == NULL) {
        return NULL;
    }
    data = malloc(num_bytes);
    if (data == NULL) {
        free(bp);
        return NULL;
    }
    memset(data, 0, num_bytes);
    bp->size      = 0;
    bp->data_size = num_bytes;
    bp->data      = data;
    return bp;
}

bitpack_t
bitpack_init_from_bytes(const char *bytes, unsigned long num_bytes) {
    bitpack_t bp;

    bp = bitpack_init(num_bytes);
    if (bp == NULL) {
        return NULL;
    }
    memcpy(bp->data, bytes, num_bytes);
    bp->size = num_bytes * 8;
    return bp;
}

void
bitpack_destroy(bitpack_t bp) {
    free(bp->data);
    free(bp);
}

unsigned long
bitpack_size(bitpack_t bp) {
    return bp->size;
}

unsigned long
bitpack_data_size(bitpack_t bp) {
    return bp->data_size;
}

int
bitpack_on(bitpack_t bp, unsigned long index) {
    unsigned long byte_offset;
    unsigned long bit_offset;

    if (bitpack_size(bp) == 0 || index > bitpack_size(bp) - 1) {
        if (!_bitpack_resize(bp, index + 1)) {
            return BITPACK_ERR_MALLOC_FAILED;
        }
    }
    byte_offset = index / 8;
    bit_offset  = index % 8;
    bp->data[byte_offset] |= (0x80 >> bit_offset);
    return BITPACK_RV_SUCCESS;
}

int
bitpack_off(bitpack_t bp, unsigned long index) {
    unsigned long byte_offset;
    unsigned long bit_offset;

    if (bitpack_size(bp) == 0 || index > bitpack_size(bp) - 1) {
        if (!_bitpack_resize(bp, index + 1)) {
            return BITPACK_ERR_MALLOC_FAILED;
        }
    }
    byte_offset = index / 8;
    bit_offset  = index % 8;
    bp->data[byte_offset] &= ~(0x80 >> bit_offset);
    return BITPACK_RV_SUCCESS;
}

int
bitpack_get(bitpack_t bp, unsigned long index, unsigned char *bit) {
    unsigned long byte_offset;
    unsigned long bit_offset;

    if (bitpack_size(bp) == 0) {
        print("bitpack is empty\n");
        return BITPACK_ERR_EMPTY;
    }
    if (index > bitpack_size(bp) - 1) {
        print("invalid index (%lu), max index is %lu\n", index, bitpack_size(bp) - 1);
        return BITPACK_ERR_INVALID_INDEX;
    }

    byte_offset = index / 8;
    bit_offset  = index % 8;
    if (bp->data[byte_offset] & (0x80 >> bit_offset)) {
        *bit = 1;
    }
    else {
        *bit = 0;
    }
    return BITPACK_RV_SUCCESS;
}

static int
bitpack_set_bits(bitpack_t bp, unsigned long value, unsigned long num_bits,
        unsigned long index) {
    unsigned long i;
    unsigned long mask;

    /* make sure the range isn't bigger than the size of an unsigned long */
    if (num_bits > sizeof(unsigned long) * 8) {
        print("range size %lu bits is too large (maximum size is %lu bits)\n",
                num_bits, sizeof(unsigned long) * 8);
        return BITPACK_RV_ERROR;
    }

    /* make sure that the range is large enough to pack value */
    if (value > pow(2, num_bits) - 1) {
        print("value %lu does not fit in %lu bits\n", value, num_bits);
        return BITPACK_RV_ERROR;
    }

    if (bitpack_size(bp) < index + num_bits) {
        if (!_bitpack_resize(bp, index + num_bits)) {
            return BITPACK_RV_ERROR;
        }
    }

    for (i = num_bits; i != 0; i--) {
        mask = 1 << (i - 1);
        if (value & mask) {
            if (!bitpack_on(bp, index)) {
                return BITPACK_RV_ERROR;
            }
        }
        else {
            if (!bitpack_off(bp, index)) {
                return BITPACK_RV_ERROR;
            }
        }
        index++;
    }
    return BITPACK_RV_SUCCESS;
}

int
bitpack_set_bytes(bitpack_t bp, const char *value, unsigned long num_bytes,
        unsigned long index) {
    unsigned long i;

    if (bitpack_size(bp) < index + num_bytes * 8) {
        if (!_bitpack_resize(bp, index + num_bytes * 8)) {
            return BITPACK_ERR_MALLOC_FAILED;
        }
    }

    if (index % 8 == 0) {
        /* index is at the beginning of a byte, so just do a memcpy */
        memcpy(bp->data + index / 8, value, num_bytes);
    }
    else {
        /* need to set each bit individually */
        for (i = 0; i < num_bytes; i++) {
            if (!bitpack_set_bits(bp, value[i], 8, index + i * 8)) {
                return BITPACK_ERR_VALUE_WRONG;
            }
        }
    }
    return BITPACK_RV_SUCCESS;
}

static int
bitpack_get_bits(bitpack_t bp, unsigned long num_bits, unsigned long index,
        unsigned char *value) {
    unsigned long i, v = 0;
    unsigned char bit;

    if (index >= bitpack_size(bp)) {
        print("invalid index (%lu), max index is %lu\n", index, bitpack_size(bp) - 1);
        return BITPACK_RV_ERROR;
    }

    if (index + num_bits > bitpack_size(bp)) {
        print("attempted to read past end of bitpack (last index is %lu)\n",
                bitpack_size(bp) - 1);
        return BITPACK_RV_ERROR;
    }

    if (num_bits > sizeof(unsigned long) * 8) {
        print("range size %lu bits is too large (maximum size is %lu bits)\n",
                num_bits, sizeof(unsigned long) * 8);
        return BITPACK_RV_ERROR;
    }

    for (i = 0; i < num_bits; i++) {
        bitpack_get(bp, index + i, &bit);
        if (bit == 1) {
            v |= bit << (num_bits - i - 1);
        }
    }
    *value = v;
    return BITPACK_RV_SUCCESS;
}

int
bitpack_get_bytes(bitpack_t bp, unsigned long num_bytes, unsigned long index,
        luaL_Buffer *b) {
    unsigned long i;
    unsigned char byte;

    if (index >= bitpack_size(bp)) {
        print("invalid index (%lu), max index is %lu\n", index, bitpack_size(bp) - 1);
        return BITPACK_ERR_INVALID_INDEX;
    }

    if (index + num_bytes * 8 > bitpack_size(bp)) {
        print("attempted to read past end of bitpack (last index is %lu)\n",
                bitpack_size(bp) - 1);
        return BITPACK_ERR_RANGE_TOO_BIG;
    }

    if (index % 8 == 0) {
        /* index is the start of a byte, so just add */
        luaL_addlstring(b, (const char *)bp->data + index / 8, num_bytes);
    }
    else {
        /* need to unpack a byte at a time */
        for (i = 0; i < num_bytes; i++) {
            bitpack_get_bits(bp, 8, index + i * 8, &byte);
            luaL_addchar(b, byte);
        }
    }
    return BITPACK_RV_SUCCESS;
}

unsigned long
bitpack_to_bytes(bitpack_t bp, luaL_Buffer *b) {
    unsigned long bytes_size = round8(bp->size) / 8;
    luaL_addlstring(b, (const char *)bp->data, bytes_size);
    return bytes_size;
}
