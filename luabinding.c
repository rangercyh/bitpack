#ifndef _LUA_BITPACK_H_
#define _LUA_BITPACK_H_

#include <lua.h>
#include <lauxlib.h>
#include "bitpack.h"

#define MT_NAME ("_bitpack_metatable")

static int
size(lua_State *L) {
    lua_getiuservalue(L, 1, 1);
    bitpack_t bp = lua_touserdata(L, -1);
    unsigned long bits_size = bitpack_size(bp);
    lua_pushinteger(L, bits_size);
    return 1;
}

static int
alloc_bytes(lua_State *L) {
    lua_getiuservalue(L, 1, 1);
    bitpack_t bp = lua_touserdata(L, -1);
    unsigned long mem_size = bitpack_data_size(bp);
    lua_pushinteger(L, mem_size);
    return 1;
}

static int
on(lua_State *L) {
    int pos = luaL_checkinteger(L, 2);
    if (pos < 0) {
        luaL_error(L, "on pos (%d) must >= 0", pos);
    }
    lua_getiuservalue(L, 1, 1);
    bitpack_t bp = lua_touserdata(L, -1);
    lua_pushboolean(L, bitpack_on(bp, pos) == BITPACK_RV_SUCCESS);
    return 1;
}

static int
off(lua_State *L) {
    int pos = luaL_checkinteger(L, 2);
    if (pos < 0) {
        luaL_error(L, "on pos (%d) must >= 0", pos);
    }
    lua_getiuservalue(L, 1, 1);
    bitpack_t bp = lua_touserdata(L, -1);
    lua_pushboolean(L, bitpack_off(bp, pos) == BITPACK_RV_SUCCESS);
    return 1;
}

static int
get(lua_State *L) {
    int pos = luaL_checkinteger(L, 2);
    if (pos < 0) {
        luaL_error(L, "on pos (%d) must >= 0", pos);
    }

    lua_getiuservalue(L, 1, 1);
    bitpack_t bp = lua_touserdata(L, -1);
    unsigned char bit;
    int rv = bitpack_get(bp, pos, &bit);
    lua_pushboolean(L, rv == BITPACK_RV_SUCCESS);
    if (rv == BITPACK_RV_SUCCESS) {
        lua_pushboolean(L, 1);
        lua_pushinteger(L, bit);
    } else {
        lua_pushboolean(L, 0);
        lua_pushinteger(L, rv);
    }
    return 2;
}

static int
set_bytes(lua_State *L) {
    int pos = luaL_checkinteger(L, 2);
    if (pos < 0) {
        luaL_error(L, "set_bytes pos (%d) must >= 0", pos);
    }
    size_t len;
    const char *bytes = luaL_checklstring(L, 3, &len);
    if (!bytes) {
        luaL_error(L, "set_bytes bytes string is nil");
    }

    lua_getiuservalue(L, 1, 1);
    bitpack_t bp = lua_touserdata(L, -1);
    int rv = bitpack_set_bytes(bp, bytes, len, pos);
    if (rv == BITPACK_RV_SUCCESS) {
        lua_pushboolean(L, 1);
        return 1;
    }
    lua_pushboolean(L, 0);
    lua_pushinteger(L, rv);
    return 2;
}

static int
get_bytes(lua_State *L) {
    int pos = luaL_checkinteger(L, 2);
    if (pos < 0) {
        luaL_error(L, "get_bytes pos (%d) must >= 0", pos);
    }
    int num_bytes = luaL_checkinteger(L, 3);
    if (num_bytes <= 0) {
        luaL_error(L, "get_bytes number (%d) must > 0", num_bytes);
    }

    luaL_Buffer b;
    luaL_buffinit(L, &b);
    lua_getiuservalue(L, 1, 1);
    bitpack_t bp = lua_touserdata(L, -1);
    int rv = bitpack_get_bytes(bp, num_bytes, pos, &b);
    if (rv == BITPACK_RV_SUCCESS) {
        luaL_pushresult(&b);
        return 1;
    } else {
        lua_pushnil(L);
        lua_pushinteger(L, rv);
        return 2;
    }
}

static int
append_bytes(lua_State *L) {
    size_t len;
    const char *bytes = luaL_checklstring(L, 2, &len);
    if (!bytes) {
        luaL_error(L, "set_bytes bytes string is nil");
    }

    lua_getiuservalue(L, 1, 1);
    bitpack_t bp = lua_touserdata(L, -1);
    int rv = bitpack_append_bytes(bp, bytes, len);
    if (rv == BITPACK_RV_SUCCESS) {
        lua_pushboolean(L, 1);
        return 1;
    }
    lua_pushboolean(L, 0);
    lua_pushinteger(L, rv);
    return 2;
}

static int
to_bytes(lua_State *L) {
    luaL_Buffer b;
    luaL_buffinit(L, &b);
    lua_getiuservalue(L, 1, 1);
    bitpack_t bp = lua_touserdata(L, -1);
    bitpack_to_bytes(bp, &b);
    luaL_pushresult(&b);
    return 1;
}

static int
gc(lua_State *L) {
    lua_getiuservalue(L, 1, 1);
    if (lua_islightuserdata(L, -1)) {
        bitpack_destroy(lua_touserdata(L, -1));
    }
    return 0;
}

static int
lmetatable(lua_State *L) {
    if (luaL_newmetatable(L, MT_NAME)) {
        luaL_Reg l[] = {
            {"size", size},             // size of the max set bit
            {"alloc_bytes", alloc_bytes}, // bytes of allocated

            {"on", on},     // set 1
            {"off", off},   // set 0
            {"get", get},   // get 0/1

            {"set_bytes", set_bytes},       // set bytes data to bp
            {"get_bytes", get_bytes},       // get bytes array
            {"append_bytes", append_bytes}, // append bytes data to bp
            {"to_bytes", to_bytes},         // dump bp to bytes array

            { NULL, NULL }
        };
        luaL_newlib(L, l);
        lua_setfield(L, -2, "__index");

        lua_pushcfunction(L, gc);
        lua_setfield(L, -2, "__gc");
    }
    return 1;
}

static int
lnew(lua_State *L) {
    bitpack_t bp = NULL;
    if (lua_type(L, 1) == LUA_TNUMBER) {
        int num_bytes = luaL_checkinteger(L, 1);
        if (num_bytes <= 0) {
            luaL_error(L, "num_bytes (%d) must > 0", num_bytes);
        }
        bp = bitpack_init(num_bytes);
    } else if (lua_type(L, 1) == LUA_TSTRING) {
        size_t len;
        const char *bytes = luaL_checklstring(L, 1, &len);
        bp = bitpack_init_from_bytes(bytes, len);
    } else {
        bp = bitpack_init_default();
    }
    if (!bp) {
        luaL_error(L, "init bitpack failed");
    }

    lua_newuserdatauv(L, 0, 1);
    lua_pushlightuserdata(L, bp);
    lua_setiuservalue(L, -2, 1);
    lmetatable(L);
    lua_setmetatable(L, -2);
    return 1;
}

LUAMOD_API int
luaopen_bitpack(lua_State *L) {
    luaL_checkversion(L);
    luaL_Reg l[] = {
        { "new", lnew },
        { NULL, NULL },
    };
    luaL_newlib(L, l);
    return 1;
}

#endif
