#ifndef COMPONENT_DB_H
#define COMPONENT_DB_H

#include "Lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"
#include "rapidjson/document.h"

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <iostream>

class ComponentDB
{
public:
	//establishes global state, reads in and catalogues all component files
	static void init();
	static void deinit();

	//initializes instance_table as an instance of type comp_name
	static int get_component(luabridge::LuaRef& instance_table, std::string comp_name);

	//reads in json overrides for a lua component instance
	static int read_component_json_A2(luabridge::LuaRef* component_table, const rapidjson::Value& comp_json);
	static int read_component_json(luabridge::LuaRef* component_table, const rapidjson::Value& comp_json);

	static int inherit(luabridge::LuaRef& instance_table, const luabridge::LuaRef& parent_table);

	static lua_State* get_state() { return state; }

	static bool is_init() { return initialized; }

private:
	inline static lua_State* state;
	inline static bool initialized = false;

	static void add_global_classes();
	static void add_global_functions();

	static void check_init();
};

class LuaFuncs {
public:
	static void cpp_log(const std::string& message) {
		std::cout << message << std::endl;
	}

	static void cpp_log_err(const std::string& message) {
		std::cerr << message << std::endl;
	}

	static void cpp_quit() {
		exit(0);
	}

	static void cpp_sleep(int ms);
	static int cpp_getframe();
	static void cpp_open(const std::string& url);

	static bool cpp_input_key(const std::string& code);
	static bool cpp_input_keydown(const std::string& code);
	static bool cpp_input_keyup(const std::string& code);
};

#endif