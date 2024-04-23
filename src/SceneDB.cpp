#include "SceneDB.h"

#include <filesystem>
#include <iostream>
#include <algorithm>
#include <queue>

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"

#include "EngineUtils.h"
#include "Consts.h"
#include "Renderer.h"
#include "Helper.h"
#include "AudioDB.h"
#include "ComponentDB.h"

using std::cout;
using std::endl;
using std::shared_ptr;

void SceneDB::init(rapidjson::Document& d)
{
	if (initialized) {
		cout << "error: double Renderer init call";
		exit(0);
	}

	if (!TemplateDB::is_init())
		TemplateDB::init();

	initialized = true;

	load_scene(d);
}

void SceneDB::deinit()
{
	if (!initialized) {
		cout << "error: tried to deinit an uninitialized Renderer";
		exit(0);
	}

	actors.clear();
	get_return_vector.clear();

	initialized = false;
}

void SceneDB::load_scene(rapidjson::Document& d)
{
	check_init();

	//note: super terrible at checking for errors
	for (auto& layer_obj : d["layers"].GetArray()) {
		//skip iter if layer_obj is badly formatted
		if (!(layer_obj.HasMember("name") && layer_obj["name"].IsString())) continue;

		std::string layer_name = layer_obj["name"].GetString();

		if (layer_name == LAYER_OBJECTS_NAME) {
			init_actors(layer_obj);
		}


		else if (layer_name == LAYER_MAP_NAME) {

			//read and init tilemap

			int id = layer_obj["id"].GetInt();
			std::string map_name = "";
			for (size_t i = 0; i < d["tilesets"].GetArray().Size(); ++i) {
				if (d["tilesets"].GetArray()[i]["firstgid"].GetInt() == id) {
					//remember filename (without directory) of map file
					map_name = std::filesystem::path(d["tilesets"].GetArray()[i]["source"].GetString()).filename().string();
					break;
				}
			}

			map = std::make_shared<Tilemap>(layer_obj, map_name);

		}
	}
}

bool SceneDB::tick()
{
	check_init();

	if (map.get() != nullptr)
		map->draw();


	for (shared_ptr<Actor>& a : running_actors) {
		a->start();
	}

	for (shared_ptr<Actor>& a : running_actors) {
		a->update();
	}

	for (shared_ptr<Actor>& a : running_actors) {
		a->late_update();
	}

	//add new actors to running list
	for (shared_ptr<Actor>& a : new_actors) {
		running_actors.emplace_back(a);
	}
	new_actors.clear();

	//clean up any destroyed actors
	to_destroy.clear();

	return false;
}

void SceneDB::keep_actor(const luabridge::LuaRef& actor)
{
	Actor* a = actor;
	shared_ptr<Actor> local = nullptr;
	for (size_t i = 0; i < actors.size(); ++i) {
		if (actors[i]->id == a->id) {
			local = actors[i];
			local->scene_persist = true;
			break;
		}
	}
}

Actor* SceneDB::get_actor(const std::string& name)
{
	for (shared_ptr<Actor> a : actors) {
		if (a->name == name) return a.get();
	}
	return nullptr;
}

std::vector<Actor*>& SceneDB::get_actors(const std::string& name)
{
	get_return_vector.clear();
	for (shared_ptr<Actor> a : actors) {
		if (a->name == name) get_return_vector.push_back(a.get());
	}
	return get_return_vector;
}

luabridge::LuaRef SceneDB::cpp_instantiate(const std::string templ_name)
{
	const Actor* temp = TemplateDB::get_template_actor(templ_name);
	actors.emplace_back(new Actor(temp));
	shared_ptr<Actor> a = actors.back();
	new_actors.emplace_back(a);
	return luabridge::LuaRef(ComponentDB::get_state(), a.get());
}

void SceneDB::cpp_destroy(const luabridge::LuaRef& actor)
{
	Actor* a = actor;
	shared_ptr<Actor> local = nullptr;
	for (size_t i = 0; i < actors.size(); ++i) {
		if (actors[i]->id == a->id) {
			local = actors[i];
			actors.erase(actors.begin() + i);
			local->set_active(false);
			to_destroy.push_back(local);
			break;
		}
	}
}


/* ------------------------ private ------------------------ */

void SceneDB::check_init() {
	if (!initialized) {
		cout << "error: called SceneDB function before initializing";
		exit(0);
	}
}

void SceneDB::init_actors(rapidjson::Value& actor_layer)
{
	//empty all structures
	std::vector<std::shared_ptr<Actor>> persistent;
	for (auto& actor_ptr : actors) {
		if (actor_ptr->scene_persist)
			persistent.push_back(actor_ptr);
	}
	actors.clear();
	to_destroy.clear();

	//check actor_layer is formatted correctly
	if (!(actor_layer.HasMember("objects") && actor_layer["objects"].IsArray())) {
		cout << "Error: tried to read in actors from layer " << LAYER_OBJECTS_NAME << " but could not find an objects array";
		exit(0);
	}

	//initialize actors vector with its full size (to avoid actors moving around)
	int size = actor_layer["objects"].GetArray().Size() + persistent.size();
	actors.reserve(size);

	//place persistent actors in actors vector
	actors = persistent;

	//load all actors into actors vector
	for (auto& actor : actor_layer["objects"].GetArray()) {

		actors.emplace_back(new Actor(actor));

		shared_ptr<Actor> a = actors.back();

		running_actors.emplace_back(a);

		//do things with a
	}
}
