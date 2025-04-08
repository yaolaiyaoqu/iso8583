#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "iso8583.h"

#if LUA_VERSION_NUM <= 501 

#define lua_rawlen(L, index)      lua_objlen(L, index)
#define luaL_setfuncs(L, l, nup)  luaL_register(L, NULL, l)
#define luaL_newlib(L, l)         lua_newtable(L); luaL_register(L, NULL, l);

#endif

typedef struct iso8583_userdata {
	struct iso8583 *handle;
} iso8583_userdata;

static int check_size_define(lua_State *L, int index, int *size)
{
	lua_getfield(L, -1, "size");
	
	if (!lua_isnumber(L, -1)) {
		lua_pushnil(L);
		lua_pushfstring(L, "field %d error! the size is not a number!", index);
		return 0;
	}

	*size = lua_tointeger(L, -1);
	lua_pop(L, 1);

	return 1;
}


static int check_type_define(lua_State *L, int index, int *type)
{
	lua_getfield(L, -1, "type");

	if (!lua_isnumber(L, -1)) {
		lua_pushnil(L);
		lua_pushfstring(L, "field %d error! the type is invalid!", index);
		return 0;
	}
	
	*type = lua_tointeger(L, -1);
	lua_pop(L, 1);

	return 1;
}

static int check_align_define(lua_State *L, int index, int *align)
{
	lua_getfield(L, -1, "align");

	if (!lua_isnumber(L, -1)) {
		lua_pushnil(L);
		lua_pushfstring(L, "field %d error! the align is invalid!", index);
		return 0;
	}
	
	*align = lua_tointeger(L, -1);
	lua_pop(L, 1);

	return 1;	
}

static int check_compress_define(lua_State *L, int index, int *compress)
{
	lua_getfield(L, -1, "compress");

	if (!lua_isnumber(L, -1)) {
		lua_pushnil(L);
		lua_pushfstring(L, "field %d error! the compress is invalid!", index);
		return 0;
	}
	
	*compress = lua_tointeger(L, -1);
	lua_pop(L, 1);

	return 1;
}

static int check_pad_define(lua_State *L, int index, char *pad)
{
	lua_getfield(L, -1, "pad");

	if (!lua_isstring(L, -1) || !lua_rawlen(L, -1)) {
		lua_pushnil(L);
		lua_pushfstring(L, "field %d error! the pad is invalid!", index);
		return 0;
	}
	
	*pad = *lua_tostring(L, -1);
	lua_pop(L, 1);

	return 1;
}

static int lua_iso8583_new(lua_State *L)
{
	struct iso8583 *iso8583h;
	iso8583_userdata *iso8583u = (iso8583_userdata *)lua_newuserdata(L, sizeof(iso8583_userdata));
	iso8583u->handle = NULL;

	luaL_getmetatable(L, "iso8583");
	lua_setmetatable(L, -2);

	iso8583h = iso8583u->handle = iso8583_create();

	if (!iso8583h) {
		lua_pushnil(L);
		lua_pushstring(L, "alloc memory failed!");
		return 2;
	}

	if (!lua_istable(L, 1))
		return 1;

	lua_pushnil(L);  

	while (lua_next(L, 1) != 0)	{

		if (!lua_isnumber(L, -2)) {
			lua_pop(L, 1);
			continue;
		}

		int i = lua_tointeger(L, -2);

		if (i >= 0 && i <= 128 && lua_istable(L, -1)) {
			int size, type, align, compress;
			char pad;

			if (!check_size_define(L, i, &size)) 
				return 2;

			if (!check_type_define(L, i, &type))
				return 2;

			if (!check_align_define(L, i, &align))
				return 2;

			if (!check_compress_define(L, i, &compress))
				return 2;

			if (!check_pad_define(L, i, &pad))
				return 2;

			if (iso8583_define(iso8583h, i, (unsigned int)size, pad, (unsigned int)type, 
					(unsigned int)align, (unsigned int)compress) != ISO8583_OK) {
				lua_pushnil(L);
				lua_pushfstring(L, "field %u error! the field is invalid! size = %u, "
						"pad = %c, type = %u, align = %u, compress = %u!",
						i, size, pad, type, align, compress);
				return 2;
			}
		}

		lua_pop(L, 1);
	}

	return 1;
}

static int lua_iso8583_pack(lua_State *L)
{
	iso8583_userdata *iso8583u;
	int n = lua_gettop(L);
	unsigned int size;
	unsigned char *iso8583_data;

	if (n < 2) {
		lua_pushnil(L);
		lua_pushstring(L, "Arguments error! The data must be exist!");
		return 2;
	}

	iso8583u = (iso8583_userdata *)luaL_checkudata(L, 1, "iso8583");

	if (!lua_istable(L, 2)) {
		lua_pushnil(L);
		lua_pushstring(L, "Argument error! The data must be table!");
		return 2;
	} 

	iso8583_clear_datas(iso8583u->handle);

	lua_pushnil(L);

	while(lua_next(L, 2) != 0) {

		if (!lua_isnumber(L, -2)) {
			lua_pop(L, 1);
			continue;
		}

		int i = lua_tointeger(L, -2);
		
		if (i >= 0 && i <= 128) {
			size_t size;
			const unsigned char *data;

			if (!lua_isstring(L, -1)) {
				lua_pushnil(L);
				lua_pushfstring(L, "data %d error! is not a string!", i);
				return 2;
			}

			data = (const unsigned char *)lua_tolstring(L, -1, &size);

			if (iso8583_set(iso8583u->handle, i, data, (unsigned int)size) != ISO8583_OK) {
				lua_pushnil(L);
				lua_pushfstring(L, "data %d error!", i);
				return 2;
			}
		}

		lua_pop(L, 1);
	}

	lua_pop(L, 1);

	iso8583_size(iso8583u->handle, &size);
	iso8583_data = malloc(size);

	if (iso8583_data == NULL) {
		lua_pushnil(L);
		lua_pushstring(L, "Pack iso8583 error! alloc memory failed!");
		return 2;
	}

	if (iso8583_pack(iso8583u->handle, iso8583_data, &size) != ISO8583_OK) {
		lua_pushnil(L);
		lua_pushfstring(L, "Pack iso8583 error! %s", iso8583u->handle->error);
		free(iso8583_data);
		return 2;
	}

	lua_pushlstring(L, (const char *)iso8583_data, size);
	free(iso8583_data);
	return 1;
}

static int lua_iso8583_unpack(lua_State *L)
{
	iso8583_userdata *iso8583u;
	unsigned char *iso8583_data;
	unsigned int iso8583_size;
	unsigned int maxfield;
	size_t data_len;
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

	if (n >= 3) {
		maxfield = (unsigned int )lua_tonumber(L, 3);

		if (maxfield > 128 ) {
			lua_pushnil(L);
			lua_pushstring(L, "arguments error! maxfield must in [2,128]!");
			return 2;
		}
	} else {
		maxfield = 128;
	}

	iso8583_clear_datas(iso8583u->handle);
	iso8583_data = (unsigned char *)lua_tolstring(L, 2, &data_len);
	iso8583_size = (unsigned int)data_len;

	if (iso8583_unpack(iso8583u->handle, iso8583_data, &iso8583_size, maxfield) != ISO8583_OK) {
		lua_pushnil(L);
		lua_pushfstring(L, "unpack iso8583 error! %s", iso8583u->handle->error);
		return 2;
	}

	lua_createtable (L, 129, 0);

	for (i = 0; i < 129 && i <= maxfield; i++) {
		const unsigned char *user_data;
		unsigned int user_size;

		if (iso8583_get(iso8583u->handle, i, &user_data, &user_size) != ISO8583_OK) {
			lua_pushnil(L);
			lua_pushfstring(L, "unpack iso8583 error! %s", iso8583u->handle->error);
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
	{ "new",    lua_iso8583_new    },
	{ NULL, NULL}
};

static const struct luaL_Reg iso8583lib_m[] = {
	{ "Pack",   lua_iso8583_pack   },
	{ "Unpack", lua_iso8583_unpack },
	{ "__gc",   lua_iso8583_close  },
	{ NULL, NULL}
};

int luaopen_iso8583(lua_State *L)
{
	luaL_newmetatable(L, "iso8583");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");

	luaL_setfuncs(L, iso8583lib_m, 0);
	luaL_setfuncs(L, iso8583lib_f, 0);

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
	set_constant("LLVAR_U",  ISO8583_LLVAR_U);
	set_constant("LLLVAR_U", ISO8583_LLLVAR_U);

	return 1;
}

