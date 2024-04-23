#include "SceneDB.h"

#include <filesystem>
#include <iostream>
#include <algorithm>
#include <queue>

#include "rapidjson/document.h"

#include "EngineUtils.h"
#include "Consts.h"
#include "Renderer.h"
#include "Helper.h"
#include "AudioDB.h"
#include "ComponentDB.h"

using std::cout;
using std::endl;
using std::shared_ptr;

void SceneDB::init()
{
	if (initialized) {
		cout << "error: double Renderer init call";
		exit(0);
	}

	if (!TemplateDB::is_init())
		TemplateDB::init();

	initialized = true;
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

void SceneDB::load_scene(std::string scene)
{
	check_init();

	//empty all structures
	std::vector<std::shared_ptr<Actor>> persistent;
	for (auto& actor_ptr : actors) {
		if (actor_ptr->scene_persist)
			persistent.push_back(actor_ptr);
	}
	actors.clear();
	to_destroy.clear();

	//check that scene file exists
	std::string scene_path = SCENES_FOLDER_PATH + scene + ".scene";

	if (!std::filesystem::exists(scene_path)) {
		cout << "error: scene " << scene << " is missing";
		exit(0);
	}

	//load scene file into Document d
	rapidjson::Document d;
	EngineUtils::ReadJsonFile(scene_path, d);

	//exit early if there is no "actors" member of the scene file
	if (!(d.HasMember("actors") && d["actors"].IsArray())) {
		return;
	}

	//initialize actors vector with its full size (to avoid actors moving around)
	int size = d["actors"].GetArray().Size() + persistent.size();
	actors.reserve(size);

	//place persistent actors in actors vector
	actors = persistent;

	//load all actors into actors vector
	for (rapidjson::Value& actor : d["actors"].GetArray()) {

		const Actor* temp = nullptr;

		//if new actor is templated, construct actor with template
		if (actor.HasMember("template") && actor["template"].IsString())
			temp = TemplateDB::get_template_actor(actor["template"].GetString());

		actors.emplace_back(new Actor(actor, temp, false));

		shared_ptr<Actor> a = actors.back();

		running_actors.emplace_back(a);

		//do things with a
	}
}

bool SceneDB::tick()
{
	check_init();


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