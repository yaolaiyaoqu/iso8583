#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "iso8583.h"

#define ISO8583_MAXSIZE 81920

static void hexdump(const unsigned char *s, int l)
{
	int n = 0;

	for (; n < l; ++n) {
		printf("%02x", s[n]);
	}

	printf("\n");
}

typedef struct iso8583_userdata {
	struct iso8583 *handle;
} iso8583_userdata;

static int lua_iso8583_new(lua_State *L)
{
	struct iso8583 *iso8583h;
	iso8583_userdata *iso8583u = (iso8583_userdata *)lua_newuserdata(L, sizeof(iso8583_userdata));
	char error[BUFSIZ];

	iso8583u->handle = NULL;

	luaL_getmetatable(L, "iso8583");
	lua_setmetatable(L, -2);

	iso8583h = iso8583u->handle = iso8583_create();

	if (!iso8583h) {
		lua_pushnil(L);
		lua_pushstring(L, "alloc memory failed!");
		return 2;
	}

	if (lua_istable(L, 1)) {
		lua_pushnil(L);  
		while (lua_next(L, 1) != 0)	{
			if (lua_isnumber(L, -2)) {
				int i = lua_tointeger(L, -2);

				if (i >= 0 && i <= 128) {
					if (lua_istable(L, -1)) {
						int size;
						int type;
						int align;
						int compress;
						char pad;
						
						// get size
						lua_getfield(L, -1, "size");
						
						if (!lua_isnumber(L, -1)) {
							lua_pushnil(L);
							snprintf(error, BUFSIZ, "field %d error! the size is not a number!", i);
							lua_pushstring(L, error);
							return 2;
						}

						size = lua_tointeger(L, -1);

						if (size < 0) {
							lua_pushnil(L);
							snprintf(error, BUFSIZ, "field %d error! the size is little then 0! size = %d", i, size);
							lua_pushstring(L, error);
							return 2;
						}

						lua_pop(L, 1);

						// get type
						lua_getfield(L, -1, "type");

						if (!lua_isnumber(L, -1)) {
							lua_pushnil(L);
							snprintf(error, BUFSIZ, "field %d error! the type is invalid!", i);
							lua_pushstring(L, error);
							return 2;
						}
						
						type = lua_tointeger(L, -1);

						lua_pop(L, 1);

						// get align;
						lua_getfield(L, -1, "align");

						if (!lua_isnumber(L, -1)) {
							lua_pushnil(L);
							snprintf(error, BUFSIZ, "field %d error! the align is invalid!", i);
							lua_pushstring(L, error);
							return 2;
						}
						
						align = lua_tointeger(L, -1);

						lua_pop(L, 1);

						// get compress;
						lua_getfield(L, -1, "compress");

						if (!lua_isnumber(L, -1)) {
							lua_pushnil(L);
							snprintf(error, BUFSIZ, "field %d error! the compress is invalid!", i);
							lua_pushstring(L, error);
							return 2;
						}
						
						compress = lua_tointeger(L, -1);

						lua_pop(L, 1);

						// get pad;
						lua_getfield(L, -1, "pad");

						if (!lua_isstring(L, -1) || !lua_objlen(L, -1)) {
							lua_pushnil(L);
							snprintf(error, BUFSIZ, "field %d error! the pad is invalid!", i);
							lua_pushstring(L, error);
							return 2;
						}
						
						pad = *lua_tostring(L, -1);

						lua_pop(L, 1);

						// define field
						if (iso8583_define(iso8583h, i, (unsigned int)size, pad, (unsigned int)type, 
											(unsigned int)align, (unsigned int)compress) != ISO8583_OK) {
							lua_pushnil(L);
							snprintf(error, BUFSIZ, "field %u error! the field is invalid! size = %u, "
													"pad = %c, type = %u, align = %u, compress = %u!", 
										i, size, pad, type, align, compress);
							lua_pushstring(L, error);
							return 2;
						}
					}
				}
			}
			lua_pop(L, 1);
		}
	}

	return 1;
}

static int lua_iso8583_pack(lua_State *L)
{
	iso8583_userdata *iso8583u;
	char error[BUFSIZ];
	int n = lua_gettop(L);
	unsigned int iso8583_maxsize = ISO8583_MAXSIZE;
	unsigned char iso8583_data[ISO8583_MAXSIZE];

	if (n < 2) {
		lua_pushnil(L);
		lua_pushstring(L, "arguments error! too small!");
		return 2;
	}

	iso8583u = (iso8583_userdata *)luaL_checkudata(L, 1, "iso8583");

	if (!lua_istable(L, 2)) {
		lua_pushnil(L);
		lua_pushstring(L, "argument error! data must be table!");
		return 2;
	} 

	lua_pushnil(L);
	while(lua_next(L, 2) != 0) {

		if (lua_isnumber(L, -2)) {
			int i = lua_tointeger(L, -2);
			
			if (i >= 0 && i <= 128) {
				size_t size;
				const unsigned char *data;

				if (!lua_isstring(L, -1)) {
					lua_pushnil(L);
					snprintf(error, BUFSIZ, "data %d error! is not a string!", i);
					lua_pushstring(L, error);
					return 2;
				}

				data = (const unsigned char *)lua_tolstring(L, -1, &size);

				if (iso8583_set(iso8583u->handle, i, data, (unsigned int)size) != ISO8583_OK) {
					lua_pushnil(L);
					snprintf(error, BUFSIZ, "data %d error!", i);
					lua_pushstring(L, error);
					return 2;
				}
			}
		}

		lua_pop(L, 1);
	}

	lua_pop(L, 1);

	if (iso8583_pack(iso8583u->handle, iso8583_data, &iso8583_maxsize) != ISO8583_OK) {
		lua_pushnil(L);
		snprintf(error, BUFSIZ, "pack iso8583 error! %s", iso8583u->handle->error);
		lua_pushstring(L, error);
		return 2;
	}

	lua_pushlstring(L, (const char *)iso8583_data, iso8583_maxsize);
	
	return 1;
}

static int lua_iso8583_unpack(lua_State *L)
{
	iso8583_userdata *iso8583u;
	unsigned char *iso8583_data;
	unsigned int iso8583_size;
	size_t data_len;
	char error[BUFSIZ];
	unsigned int i;
	int n = lua_gettop(L);

	if (n < 2) {
		lua_pushnil(L);
		lua_pushstring(L, "arguments error! too small!");
		return 2;
	}

	iso8583u = (iso8583_userdata *)luaL_checkudata(L, 1, "iso8583");

	if (!lua_isstring(L, 2)) {
		lua_pushnil(L);
		lua_pushstring(L, "argument error! data must be string!");
		return 2;
	}

	iso8583_data = (unsigned char *)lua_tolstring(L, -1, &data_len);

	iso8583_size = (unsigned int)data_len;

	if (iso8583_unpack(iso8583u->handle, iso8583_data, &iso8583_size) != ISO8583_OK) {
		lua_pushnil(L);
		snprintf(error, BUFSIZ, "unpack iso8583 error! %s", iso8583u->handle->error);
		lua_pushstring(L, error);
		return 2;
	}

	lua_createtable (L, 129, 0);

	for (i = 0; i < 129; i++) {
		const unsigned char *user_data;
		unsigned int user_size;

		if (iso8583_get(iso8583u->handle, i, &user_data, &user_size) != ISO8583_OK) {
			lua_pushnil(L);
			snprintf(error, BUFSIZ, "unpack iso8583 error! %s", iso8583u->handle->error);
			lua_pushstring(L, error);
			return 2;
		}

		if (user_data && user_size) {
			lua_pushinteger(L, (int)i);
			lua_pushlstring(L, (const char *)user_data, user_size);

			lua_settable (L, -3);
		}
	}

	lua_pushinteger(L, (int)iso8583_size);

	return 2;
}

static int lua_iso8583_close(lua_State *L)
{
	iso8583_userdata *iso8583u = (iso8583_userdata *)luaL_checkudata(L, 1, "iso8583");

	if (iso8583u->handle)
		iso8583_destroy(iso8583u->handle);

	iso8583u->handle = NULL;

	return 0;
}

static const struct luaL_Reg iso8583lib_f[] = {
	{"new", lua_iso8583_new},
	{NULL, NULL}
};

static const struct luaL_Reg iso8583lib_m[] = {
	{"Pack", lua_iso8583_pack},
	{"Unpack", lua_iso8583_unpack},
	{"close", lua_iso8583_close},
	{"__gc", lua_iso8583_close},
	{NULL, NULL}
};

int luaopen_iso8583(lua_State *L)
{

	luaL_newmetatable(L, "iso8583");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");

	luaL_register(L, NULL, iso8583lib_m);

	luaL_register(L, "iso8583", iso8583lib_f);

#define set_constant(name, value) lua_pushstring(L, name); lua_pushinteger(L, value); lua_settable(L, -3);

	set_constant("LEFT",   ISO8583_L);
	set_constant("RIGHT",  ISO8583_R);
	set_constant("L",      ISO8583_L);
	set_constant("R",      ISO8583_R);

	set_constant("ZIP",    ISO8583_Z);
	set_constant("UNZIP",  ISO8583_U);
	set_constant("Z",      ISO8583_Z);
	set_constant("U",      ISO8583_U);

	set_constant("FIX",    ISO8583_FIX);
	set_constant("LLVAR",  ISO8583_LLVAR);
	set_constant("LLLVAR", ISO8583_LLLVAR);

	return 1;
}
