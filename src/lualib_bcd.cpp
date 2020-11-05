/*
  Copyright (c) 2020 Patrick P. Frey

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
///\file lualib_bcd.cpp
///\brief Implements the Lua ADT for BCD arithmetics
#include "bcd.hpp"
#include "export.hpp"
#include <limits>
#include <stdexcept>
extern "C" {
#include <lua.h>
#include <lauxlib.h>
}

static std::string runtimeErrorMessageWithLuaLocation( lua_State* ls, const std::runtime_error& err)
{
	lua_Debug ar;
	lua_getstack( ls, 1, &ar);
	lua_getinfo( ls, "S", &ar);
	char errstr[ 1024];
	if (ar.linedefined)
	{
		std::snprintf( errstr, sizeof(errstr), "Error in Lua script %s line %d: %s", ar.short_src, ar.linedefined, err.what());
	}
	else
	{
		std::snprintf( errstr, sizeof(errstr), "Error in Lua script %s: %s", ar.short_src, err.what());
	}
	return std::string( errstr);
}

static void lippincottFunction( lua_State* ls)
{
	try
	{
		throw;
	}
	catch (const std::runtime_error& err)
	{
		try {
			std::string msg = runtimeErrorMessageWithLuaLocation( ls, err);
			lua_pushlstring( ls, msg.c_str(), msg.size());
		}
		catch (...)
		{
			lua_pushliteral( ls, "bad alloc");
		}
		lua_error( ls);
	}
	catch (const std::bad_alloc&)
	{
		lua_pushliteral( ls, "bad alloc");
		lua_error( ls);
	}
	catch (...)
	{
		lua_pushliteral( ls, "uncaught exception");
		lua_error( ls);
	}
}


struct bcd_int_userdata_t
{
public:
	void init() noexcept
	{
		m_value.init();
	}
	void create( const char* val, std::size_t valsize)
	{
		m_value.init( val, valsize);
	}
	void create( double val)
	{
		m_value.init( val);
	}
	void destroy( lua_State* ls) noexcept
	{
		m_value.~BigInt();
	}
	static const char* metatableName() noexcept {return "bcd.int";}

	typedef bcd::BigInt ValueType;

	bcd::BigInt m_value;
};

template <class UD>
struct LuaMethods
{
	static UD* newuserdata( lua_State* ls) noexcept
	{
		UD* rt = (UD*)lua_newuserdata( ls, sizeof(UD));
		luaL_getmetatable( ls, UD::metatableName());
		lua_setmetatable( ls, -2);
		return rt;
	}
	static int create( lua_State* ls)
	{
		try
		{
			int nn = lua_gettop( ls);
			if (nn < 1) throw std::runtime_error( "too few arguments calling BCD constructor");
			if (nn > 1) throw std::runtime_error( "too many arguments calling BCD constructor");
			UD* ud = newuserdata( ls);
			switch (lua_type( ls, 1))
			{
				case LUA_TSTRING:
				{
					std::size_t len;
					const char* str = lua_tolstring( ls, 1, &len);
					ud->m_value.init( str, len);
					break;
				}
				case LUA_TNUMBER:
				{
					ud->m_value.init( lua_tonumber( ls, 1));
					break;
				}
				case LUA_TUSERDATA:
				{
					UD* operand_ud = (UD*)luaL_checkudata( ls, 1, UD::metatableName());
					ud->m_value.init( operand_ud->m_value);
					break;
				}
				default:
					throw std::runtime_error("expected STRING,NUMBER or USERDATA as argument");
			}
		}
		catch (...) { lippincottFunction( ls); }
		return 1;
	}

	static int gc( lua_State* ls)
	{
		[[maybe_unused]] static const char* functionName = "bcd:__gc";
		UD* ud = (UD*)luaL_checkudata( ls, 1, UD::metatableName());
		try
		{
			int nn = lua_gettop( ls);
			if (nn > 1) throw std::runtime_error("too many arguments calling __gc");
		}
		catch (...) { lippincottFunction( ls); }

		ud->destroy( ls);
		return 0;
	}

	static int tostring( lua_State* ls)
	{
		UD* ud = (UD*)luaL_checkudata( ls, 1, UD::metatableName());
		try
		{
			if (!lua_checkstack( ls, 6)) throw std::bad_alloc();
			int nn = lua_gettop( ls);
			if (nn > 1) throw std::runtime_error("too many arguments calling __tostring");
			std::string val = ud->m_value.tostring();
			lua_pushlstring( ls, val.c_str(), val.size());
		}
		catch (...) { lippincottFunction( ls); }
		return 1;
	}

	typedef typename UD::ValueType ValueType;

	static int cmpop( lua_State* ls, const char* functionName, bool (ValueType::*Method)( const ValueType&) const noexcept)
	{
		UD* ud = (UD*)luaL_checkudata( ls, 1, UD::metatableName());
		try
		{
			if (!lua_checkstack( ls, 6)) throw std::bad_alloc();
			int nn = lua_gettop( ls);
			if (nn < 2) throw std::runtime_error( std::string("too few arguments calling ") + functionName);
			if (nn > 2) throw std::runtime_error( std::string("too many arguments calling ") + functionName);
			switch (lua_type( ls, 2))
			{
				case LUA_TSTRING:
				{
					std::size_t len;
					const char* str = lua_tolstring( ls, 2, &len);
					bcd::BigInt operand( str, len);
					lua_pushboolean( ls, (ud->m_value.*Method)( operand));
					break;
				}
				case LUA_TNUMBER:
				{
					bcd::BigInt operand( lua_tonumber( ls, 2));
					lua_pushboolean( ls, (ud->m_value.*Method)( operand));
					break;
				}
				case LUA_TUSERDATA:
				{
					UD* operand_ud = (UD*)luaL_checkudata( ls, 2, UD::metatableName());
					lua_pushboolean( ls, (ud->m_value.*Method)( operand_ud->m_value));
					break;
				}
				default:
					throw std::runtime_error("expected STRING,NUMBER or USERDATA as argument");
			}
		}
		catch (...) { lippincottFunction( ls); }
		return 1;
	}

	static int binop( lua_State* ls, const char* functionName, ValueType (ValueType::*Method)( const ValueType&) const)
	{
		UD* ud = (UD*)luaL_checkudata( ls, 1, UD::metatableName());
		try
		{
			if (!lua_checkstack( ls, 6)) throw std::bad_alloc();
			int nn = lua_gettop( ls);
			if (nn < 2) throw std::runtime_error( std::string("too few arguments calling ") + functionName);
			if (nn > 2) throw std::runtime_error( std::string("too many arguments calling ") + functionName);
			switch (lua_type( ls, 2))
			{
				case LUA_TSTRING:
				{
					std::size_t len;
					const char* str = lua_tolstring( ls, 2, &len);
					bcd::BigInt operand( str, len);
					UD* res_ud = newuserdata( ls);
					res_ud->init();
					res_ud->m_value = (ud->m_value.*Method)( operand);
					break;
				}
				case LUA_TNUMBER:
				{
					bcd::BigInt operand( lua_tonumber( ls, 2));
					UD* res_ud = newuserdata( ls);
					res_ud->init();
					res_ud->m_value = (ud->m_value.*Method)( operand);
					break;
				}
				case LUA_TUSERDATA:
				{
					UD* operand_ud = (UD*)luaL_checkudata( ls, 2, UD::metatableName());
					UD* res_ud = newuserdata( ls);
					res_ud->init();
					res_ud->m_value = (ud->m_value.*Method)( operand_ud->m_value);
					break;
				}
				default:
					throw std::runtime_error("expected STRING,NUMBER or USERDATA as argument");
			}
		}
		catch (...) { lippincottFunction( ls); }
		return 1;
	}

	static int unop( lua_State* ls, const char* functionName, ValueType (ValueType::*Method)() const)
	{
		UD* ud = (UD*)luaL_checkudata( ls, 1, UD::metatableName());
		try
		{
			if (!lua_checkstack( ls, 6)) throw std::bad_alloc();
			int nn = lua_gettop( ls);
			if (nn < 1) throw std::runtime_error( std::string("too few arguments calling ") + functionName);
			if (nn > 2) throw std::runtime_error( std::string("too many arguments calling ") + functionName);
			// ... Lua adds a fake 2nd operand without meaning (See http://lua-users.org/lists/lua-l/2016-10/msg00347.html)
			// [Lua 5.3 User's Manual]: For the unary operators (negation, length, and bitwise not), the metamethod is computed and called with a dummy second operand, equal to the first one. This extra operand is only to simplify Lua's internals (by making these operators behave like a binary operation) and may be removed in future versions. (For most uses this extra operand is irrelevant.)
			UD* res_ud = newuserdata( ls);
			res_ud->init();
			res_ud->m_value = (ud->m_value.*Method)();
		}
		catch (...) { lippincottFunction( ls); }
		return 1;
	}

	static int add( lua_State* ls)
	{
		return binop( ls, "bcd:__add", &bcd::BigInt::add);
	}
	static int sub( lua_State* ls)
	{
		return binop( ls, "bcd:__sub", &bcd::BigInt::sub);
	}
	static int mod( lua_State* ls)
	{
		return binop( ls, "bcd:__mod", &bcd::BigInt::mod);
	}
	static int mul( lua_State* ls)
	{
		return binop( ls, "bcd:__mul", &bcd::BigInt::mul);
	}

	static int div( lua_State* ls)
	{
		[[maybe_unused]] static const char* functionName = "bcd::_div";
		UD* ud = (UD*)luaL_checkudata( ls, 1, UD::metatableName());
		try
		{
			if (!lua_checkstack( ls, 6)) throw std::bad_alloc();
			int nn = lua_gettop( ls);
			if (nn < 2) throw std::runtime_error( std::string("too few arguments calling ") + functionName);
			if (nn > 2) throw std::runtime_error( std::string("too many arguments calling ") + functionName);
			switch (lua_type( ls, 2))
			{
				case LUA_TSTRING:
				{
					std::size_t len;
					const char* str = lua_tolstring( ls, 2, &len);
					bcd::BigInt operand( str, len);
					UD* res1_ud = newuserdata( ls); res1_ud->init();
					UD* res2_ud = newuserdata( ls); res2_ud->init();
					std::pair<bcd::BigInt,bcd::BigInt> rr = (ud->m_value.div)( operand);
					res1_ud->m_value.swap( rr.first);
					res2_ud->m_value.swap( rr.second);
					break;
				}
				case LUA_TNUMBER:
				{
					bcd::BigInt operand( lua_tonumber( ls, 2));
					UD* res1_ud = newuserdata( ls); res1_ud->init();
					UD* res2_ud = newuserdata( ls); res2_ud->init();
					std::pair<bcd::BigInt,bcd::BigInt> rr = (ud->m_value.div)( operand);
					res1_ud->m_value.swap( rr.first);
					res2_ud->m_value.swap( rr.second);
					break;
				}
				case LUA_TUSERDATA:
				{
					UD* operand_ud = (UD*)luaL_checkudata( ls, 2, UD::metatableName());
					UD* res1_ud = newuserdata( ls); res1_ud->init();
					UD* res2_ud = newuserdata( ls); res2_ud->init();
					std::pair<bcd::BigInt,bcd::BigInt> rr = (ud->m_value.div)( operand_ud->m_value);
					res1_ud->m_value.swap( rr.first);
					res2_ud->m_value.swap( rr.second);
					break;
				}
				default:
					throw std::runtime_error("expected STRING,NUMBER or USERDATA as argument");
			}
		}
		catch (...) { lippincottFunction( ls); }
		return 2;
	}

	static int unm( lua_State* ls)
	{
		return unop( ls, "bcd:__unm", &bcd::BigInt::neg);
	}
	static int lt( lua_State* ls)
	{
		return cmpop( ls, "bcd:__lt", &bcd::BigInt::cmplt);
	}
	static int le( lua_State* ls)
	{
		return cmpop( ls, "bcd:__le", &bcd::BigInt::cmple);
	}
	static int eq( lua_State* ls)
	{
		return cmpop( ls, "bcd:__eq", &bcd::BigInt::cmpeq);
	}
};

static const struct luaL_Reg bcd_int_methods[] = {
	{ "__gc",		LuaMethods<bcd_int_userdata_t>::gc },
	{ "__tostring",		LuaMethods<bcd_int_userdata_t>::tostring },
	{ "__add",		LuaMethods<bcd_int_userdata_t>::add },
	{ "__sub",		LuaMethods<bcd_int_userdata_t>::sub },
	{ "__mul",		LuaMethods<bcd_int_userdata_t>::mul },
	{ "__div",		LuaMethods<bcd_int_userdata_t>::div },
	{ "div",		LuaMethods<bcd_int_userdata_t>::div },
	{ "__mod",		LuaMethods<bcd_int_userdata_t>::mod },
	{ "__unm",		LuaMethods<bcd_int_userdata_t>::unm },
	{ "__lt",		LuaMethods<bcd_int_userdata_t>::lt },
	{ "__le",		LuaMethods<bcd_int_userdata_t>::le },
	{ "__eq",		LuaMethods<bcd_int_userdata_t>::eq },
	{ nullptr,		nullptr }
};

static const struct luaL_Reg bcd_functions[] = {
	{ "int",		LuaMethods<bcd_int_userdata_t>::create },
	{ nullptr,  		nullptr }
};

static void createMetatable( lua_State* ls, const char* metatableName, const struct luaL_Reg* metatableMethods)
{
	luaL_newmetatable( ls, metatableName);
	lua_pushvalue( ls, -1);
	lua_setfield( ls, -2, "__index");
	luaL_setfuncs( ls, metatableMethods, 0);
}

extern "C" int luaopen_bcd( lua_State* ls);

DLL_PUBLIC int luaopen_bcd( lua_State* ls)
{
	createMetatable( ls, bcd_int_userdata_t::metatableName(), bcd_int_methods);
	luaL_newlib( ls, bcd_functions);
	return 1;
}

