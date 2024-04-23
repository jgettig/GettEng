#ifndef SCENE_DB_H
#define SCENE_DB_H

#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <set>
#include <memory>

#include "glm/glm.hpp"
#include "Lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"

#include "Actor.h"
#include "TemplateDB.h"

class SceneDB
{
public:

	//empty constructor, does not produce a valid SceneDB
	static void init();
	static void deinit();

	//loads scene with filename given by `scene`
	static void load_scene(const std::string scene);

	//returns whether game still wants to run
	static bool tick();

	static void keep_actor(const luabridge::LuaRef& actor);

	static Actor* get_actor(const std::string& name);
	static std::vector<Actor*>& get_actors(const std::string& name);

	static luabridge::LuaRef cpp_instantiate(const std::string templ_name);
	static void cpp_destroy(const luabridge::LuaRef& actor);


	static bool is_init() { return initialized; }

private:
	static inline std::vector<std::shared_ptr<Actor>> actors;
	static inline std::vector<Actor*> get_return_vector; //holds returned actors until next call to get_actors
	static inline std::vector<std::shared_ptr<Actor>> to_destroy;
	static inline std::vector<std::shared_ptr<Actor>> running_actors;
	static inline std::vector<std::shared_ptr<Actor>> new_actors;

	static inline bool initialized = false;

	//static inline std::map<float, std::set<int>> actor_y_map; // outer map stores y pos, inner map ordered by load order


	//static void actor_map_remove(int id, glm::vec2 pos);
	//static void actor_map_add(int id, glm::vec2 pos);

	//static glm::ivec2 get_index(glm::vec2 pos, glm::vec2 cell_size);

	static void check_init();
};

#endif