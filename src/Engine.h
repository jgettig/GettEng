#ifndef ENGINE_H
#define ENGINE_H

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

/* ----- Includes ----- */

//libraries
#include <string>
#include <queue>

//external headers
#include "glm/glm.hpp"
#include "rapidjson/document.h"
#include "Lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"

//course headers

//internal headers
#include "SceneDB.h"
#include "Actor.h"
#include "Renderer.h"
#include "AudioDB.h"
#include "ComponentDB.h"

/* ----- Consts ----- */

/* ----- Class Defns -----*/

class Engine
{
public:
	//sets up state
	Engine() /* : scene(SceneDB())*/ {}

	//main running loop of the game
	//runs input() -> update() -> render()
	void game_loop();

	static void load_scene(const std::string& new_scene);
	static luabridge::LuaRef get_scene_name() { return luabridge::LuaRef(ComponentDB::get_state(), scene_name); }


	~Engine();
private:
	inline static std::string scene_name = "";
	inline static std::string next_scene = "";

	//TODO: Replace with camera class
	bool is_running = true;
	bool won = false;
	bool lost = false;

	inline static bool do_sceneChange = true;

	//runs once at the beginning of game_loop()
	//runs game logic for beginning of play
	void onStart();

	//step that takes input from user
	void input();

	//step that updates game state based on input
	void update();

	//step that prints updated game state to the screen
	void render();
};

#endif