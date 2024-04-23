#ifndef ACTOR_H
#define ACTOR_H

/*
 * Style:
 *	var in local scope:			camelCase
 *	var outside local scope:	snake_case
 *	var in header global scope: <header>_snake_case
 *
 *	const (#define or normal)	CAP_SNAKE
 *
 *	fctn - private				camelCase()
 *	fctn - public				snake_case()
 *	fctn - inline (defn in .h)	CAP_SNAKE()
 */
#include "glm/glm.hpp"
#include "rapidjson/document.h"
#include "SDL2/SDL.h"
#include "Lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"

#include <vector>
#include <optional>
#include <string>
#include <unordered_map>
#include <map>
#include <memory>

#include "Transform.h"

class Actor
{
public:
	//default (and invalid) constructor
	Actor() {};

	//constructs an actor to be used as a template
	Actor(const rapidjson::Value& actor_in_A2, bool is_templ);

	//constructs an actor as a copy of templ, with no overrides
	Actor(const Actor* templ);

	Actor(const rapidjson::Value& actor_json);

	~Actor();

	void start() { 
		run_function(components_with_start, "OnStart"); 
		components_with_start.clear();
	}
	void update() { run_function(components_with_update, "OnUpdate"); }
	void late_update() { 
		run_function(components_with_late_update, "OnLateUpdate");
		insert_new_components();
	}

	luabridge::LuaRef get_name() { 
		return luabridge::LuaRef(state, name); 
	}
	luabridge::LuaRef get_id() { return luabridge::LuaRef(state, id); }
	luabridge::LuaRef get_component_by_key(const std::string& key) {
		if (components_by_key.count(key) != 0) return components[components_by_key[key]];
		else return luabridge::LuaRef(state);
	}
	luabridge::LuaRef get_component(const std::string& type) {
		if (components_by_type.count(type) != 0 && !components_by_type[type].empty()) return components[components_by_type[type][0]];
		else return luabridge::LuaRef(state);
	}
	luabridge::LuaRef get_components(const std::string& type) {
		luabridge::LuaRef ret_table = luabridge::newTable(state);
		if (components_by_type.count(type) != 0) {
			for (size_t i = 0; i < components_by_type[type].size(); ++i) {
				ret_table[i + 1] = components[components_by_type[type][i]];
			}
		}
		return ret_table;
	}

	static luabridge::LuaRef get_actor(const std::string& name);
	static luabridge::LuaRef get_actors(const std::string& name);

	luabridge::LuaRef cpp_add_component(const std::string& name);
	void cpp_remove_component(const luabridge::LuaRef& comp);

	void remove_all_components();

	void set_active(bool val) { active = val; }
	
	std::string name = "";
	std::string templ = "";

	bool scene_persist = false;

	int id = -1;

private:
	static inline lua_State* state = nullptr;

	std::vector<luabridge::LuaRef> new_components;	//new components as pair of <key, component>

	std::vector<luabridge::LuaRef> components;							//components ordered by load order
	std::map<std::string, size_t> components_by_key;					//component idxs ordered by key alphabetically
	std::map<std::string, std::vector<size_t>> components_by_type;		//component idxs ordered by type, then key alphabetically

	std::map<std::string, size_t> components_with_start;				//component idxs with OnStart defined, ordered by key
	std::map<std::string, size_t> components_with_update;				//component idxs with OnUpdate defined, ordered by key
	std::map<std::string, size_t> components_with_late_update;			//component idxs with OnStart defined, ordered by key

	Transform t;

	bool active = true;
	static inline int max_id = 0;
	static inline int added_components = 0;

	void run_function(std::map<std::string, size_t>& component_idxs, std::string function);

	void init_structures();
	void insert_new_components();
	void index_component(luabridge::LuaRef& comp, size_t i);

	void copy_properties(const Actor* other);

	void read_json_properties_A2(const rapidjson::Value& actor_in);
	void read_json_properties(const rapidjson::Value& actor_json, int templ_idx);

	luabridge::LuaRef& add_component(const std::string& type, const std::string& key);

	static void report_error(std::string& name, const luabridge::LuaException& e);

};



#endif