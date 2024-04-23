#include "Actor.h"

#include "Renderer.h"
#include "ImageDB.h"
#include "Consts.h"
#include "SceneDB.h"
#include "Helper.h"
#include "AudioDB.h"
#include "ComponentDB.h"

#include <cmath>
#include <iostream>
#include <algorithm>

using std::cout;
using std::endl;
using std::string;

using luabridge::LuaRef;

Actor::Actor(const rapidjson::Value& actor_in, const Actor* templ, bool is_templ) : id(max_id++)
{
	components = std::vector<luabridge::LuaRef>();
	new_components = std::vector<luabridge::LuaRef>();

	components_by_key = std::map<std::string, size_t>();
	components_by_type = std::map<std::string, std::vector<size_t>>();
	components_with_start = std::map<std::string, size_t>();
	components_with_update = std::map<std::string, size_t>();
	components_with_late_update = std::map<std::string, size_t>();

	if (state == nullptr) state = ComponentDB::get_state();

	if (templ != nullptr)
		copy_properties(templ);

	read_json_properties(actor_in);

	//index components
	for (size_t i = 0; i < components.size(); ++i) {
		LuaRef& component = components[i];
		index_component(component, i);
	}
}

Actor::Actor(const Actor* templ) : id(max_id++)
{
	if (state == nullptr) state = ComponentDB::get_state();

	if (templ != nullptr)
		copy_properties(templ);

	//index components
	for (size_t i = 0; i < components.size(); ++i) {
		LuaRef& component = components[i];
		index_component(component, i);
	}
}

Actor::~Actor()
{
	components.clear();
	new_components.clear();

	components_by_key.clear();
	components_by_type.clear();
	components_with_start.clear();
	components_with_update.clear();
	components_with_late_update.clear();
}

luabridge::LuaRef Actor::get_actor(const std::string& name) {
	Actor* a = SceneDB::get_actor(name);
	if (a == nullptr) return luabridge::LuaRef(state);
	return luabridge::LuaRef(state, *a);
}
luabridge::LuaRef Actor::get_actors(const std::string& name) {
	luabridge::LuaRef ret_table = luabridge::newTable(state);
	std::vector<Actor*>& actors = SceneDB::get_actors(name);
	for (size_t i = 0; i < actors.size(); ++i) {
		ret_table[i + 1] = actors[i];
	}
	return ret_table;
}

luabridge::LuaRef Actor::cpp_add_component(const std::string& name)
{
	luabridge::LuaRef& comp = add_component(name, "r" + std::to_string(added_components++));

	new_components.push_back(comp);

	return comp;
}

void Actor::cpp_remove_component(const luabridge::LuaRef& comp)
{
	//get and validate key
	string key = comp["key"].tostring();
	if (components_by_key.find(key) == components_by_key.end()) {
		cout << "Error: tried to remove component from actor " << name << " which is not known (by key)";
		exit(0);
	}
	components_by_key.erase(key);

	//remove from life cycle functions
	if (components_with_start.find(key) != components_with_start.end()) components_with_start.erase(key);
	if (components_with_update.find(key) != components_with_update.end()) components_with_update.erase(key);
	if (components_with_late_update.find(key) != components_with_late_update.end()) components_with_late_update.erase(key);

	//find location in components by type and remove
	bool removed = false;
	for (int i = 0; i < components_by_type[comp["type"]].size(); ++i) {
		size_t idx = components_by_type[comp["type"]][i];
		if (components[idx]["key"] == key) {
			components_by_type[comp["type"]].erase(components_by_type[comp["type"]].begin() + i);
			removed = true;
			break;
		}
	}


	if (!removed) {
		cout << "Error: tried to remove component from actor " << name << " which is not known (by type)";
		exit(0);
	}
}

void Actor::remove_all_components()
{
	for (luabridge::LuaRef& comp : components) {
		cpp_remove_component(comp);
	}
}


/* ------------------------ private ------------------------ */

void Actor::run_function(std::map<std::string, size_t>& component_idxs, std::string function)
{
	/* Call function */
	for (const auto& pair : component_idxs) {

		//check in loop because components can set our active state
		if (!active)
			break;

		LuaRef comp = components[pair.second];

		LuaRef func = comp[function];			//function ref
		LuaRef enabled = comp["enabled"];		//component enabled state ref

		try {
			// if component enabled, call OnUpdate()
			if (enabled.cast<bool>() == true)
				func(comp);
		}
		catch (const luabridge::LuaException& e) {
			report_error(name, e);
		}
	}
}

void Actor::insert_new_components()
{
	for (auto& comp : new_components) {
		size_t i = components.size();
		components.push_back(comp);
		index_component(comp, i);
	}

	new_components.clear();
}

void Actor::index_component(luabridge::LuaRef& comp, size_t i)
{
	string key = comp["key"];
	string type = comp["type"];


	components_by_key.emplace(key, i);
	components_by_type[type].emplace_back(i);

	if (comp["OnStart"].isFunction())
		components_with_start.emplace(key, i);
	if (comp["OnUpdate"].isFunction())
		components_with_update.emplace(key, i);
	if (comp["OnLateUpdate"].isFunction())
		components_with_late_update.emplace(key, i);
}

void Actor::copy_properties(const Actor* other)
{
	name = other->name;
	templ = other->name;

	/* copy components from other by key rather than by direct copy (just in case) */
	for (size_t i = 0; i < other->components.size(); ++i) {
		const LuaRef& r = other->components[i];
		if (!r["key"].isString()) {
			cout << "error: expected template actor component to know its key. Template: " << other->name;
			exit(0);
		}

		components.emplace_back(luabridge::newTable(state));
		LuaRef& new_ref = components.back();
		
		ComponentDB::inherit(new_ref, r);

		string new_key = new_ref["key"].tostring();
		string new_type = new_ref["type"];

		components_by_key.emplace(new_key, i);
		components_by_type[new_type].emplace_back(i);
	}
}

void Actor::read_json_properties(const rapidjson::Value& actor_in)
{
	if (actor_in.HasMember("name") && actor_in["name"].IsString())
		name = actor_in["name"].GetString();

	/* read in component json data if it exists (it might not if the actor purely inherits) */
	if (actor_in.HasMember("components") && actor_in["components"].IsObject()) {


		for (auto& component : actor_in["components"].GetObject()) {
			LuaRef* ref;
			string key = component.name.GetString();
			string type = "";

			//if inherited from parent template, don't make new component, copy from parent

			if (component.value.IsObject() && component.value.GetObject().HasMember("type") && component.value.GetObject()["type"].IsString()) {
				type = component.value.GetObject()["type"].GetString();
			}
			else if (components[components_by_key[key]]["type"].isString()) {
				type = components[components_by_key[key]]["type"].tostring();
			}
			else {
				cout << "undefined behevior in actor declaration";
				exit(0);
			}


			//new component
			if (components_by_key.count(key) == 0) {
				ref = &(add_component(type, key));
			}
			//overwriting inherited component
			else {
				ref = &(components[components_by_key[key]]);
			}

			ComponentDB::read_component_json(ref, component.value);

			(*ref)["actor"] = this;

		}

	}
	
}

luabridge::LuaRef& Actor::add_component(const std::string& type, const std::string& key)
{
	size_t i = components.size();

	components.emplace_back(luabridge::newTable(state));
	luabridge::LuaRef& ref = components.back();

	ComponentDB::get_component(ref, type);

	(ref)["key"] = key;
	(ref)["type"] = type;
	if ((ref)["enabled"].isNil()) (ref)["enabled"] = true;

	return ref;
}

void Actor::report_error(std::string& name, const luabridge::LuaException& e)
{
	std::string e_msg = e.what();

	/* Normalize file paths across platforms */
	std::replace(e_msg.begin(), e_msg.end(), '\\', '/');

	/* Display with color */
	std::cout << "\033[31m" << name << " : " << e_msg << "\033[0m" << std::endl;
}
