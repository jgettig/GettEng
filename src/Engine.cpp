#include "Engine.h"

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
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <cstdlib>
#include <filesystem>

//external headers
#include "glm/glm.hpp"
#include "rapidjson/document.h"
#include "Lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"

//course headers
#include "Helper.h"

//internal headers
#include "Actor.h"
#include "EngineUtils.h"
#include "Consts.h"
#include "Input.h"
#include "ComponentDB.h"



using std::cout;
using std::endl;

using std::string;

void Engine::game_loop()
{
	onStart();		//run singleton start logic

#ifdef DEBUG
	uint64_t NOW = SDL_GetPerformanceCounter();
	uint64_t LAST = 0;
	double dt = 0;
#endif

	while (is_running) {
		if (do_sceneChange) {
			load_scene();
		}

		input();
		
		update();

		render(); 

#ifdef DEBUG
		LAST = NOW;
		NOW = SDL_GetPerformanceCounter();
		dt = (double)((NOW - LAST) / (double)SDL_GetPerformanceFrequency());
		printf("%.5f seconds\ton\tframe %d\n", dt, Helper::GetFrameNumber()-1); //frame-1 since it advanced when we rendered
#endif
	}

	exit(0);
}

void Engine::load_scene()
{
	//TODO: implement

	/*
	SceneDB::load_scene(next_scene);
	scene_name = next_scene;
	next_scene.clear();

	do_sceneChange = false;
	*/
}

void Engine::load_scene_cpp(const std::string& new_scene)
{
	next_scene = new_scene;
	do_sceneChange = true;
}

Engine::~Engine()
{
	//deinitialize all static classes
	if (Renderer::is_init()) Renderer::deinit();
	if (AudioDB::is_init()) AudioDB::deinit();
	if (TemplateDB::is_init()) TemplateDB::deinit();
	if (TextDB::is_init()) TextDB::deinit();
	if (ImageDB::is_init()) ImageDB::deinit();
	if (SceneDB::is_init()) SceneDB::deinit();
	if (ComponentDB::is_init()) ComponentDB::deinit();
}

void Engine::onStart()
{
	

	/* ----- Check for resources dir ----- */
	if (!std::filesystem::exists(std::filesystem::path(RESOURCES_PATH))) {
		cout << "error: resources/ missing";
		exit(0);
	}


	/* ----- Read start config from file ----- */
	// Check for config file
	if (!std::filesystem::exists(std::filesystem::path(GAME_CONFIG_PATH))) {
		cout << "error: " << GAME_CONFIG_PATH << " missing";
		exit(0);
	}

	ComponentDB::init();

	// read messages from file and set member variables
	rapidjson::Document d;
	EngineUtils::ReadJsonFile(GAME_CONFIG_PATH, d);

	string title = "";

	//get game title
	if (d.HasMember("game_title") && d["game_title"].IsString()) 
		title = d["game_title"].GetString();

	/* ----- Read map from file ----- */

	if (d.HasMember("initial_scene") && d["initial_scene"].IsString())
		scene_name = d["initial_scene"].GetString();

	if (scene_name == "") {
		cout << "error: initial_scene unspecified";
		exit(0);
	}

	SDL_Color clear_color = { 255, 255, 255, 255 };

	if (d.HasMember("clear_color_r") && d["clear_color_r"].IsInt())
		clear_color.r = d["clear_color_r"].GetInt();
	if (d.HasMember("clear_color_g") && d["clear_color_g"].IsInt())
		clear_color.g = d["clear_color_g"].GetInt();
	if (d.HasMember("clear_color_b") && d["clear_color_b"].IsInt())
		clear_color.b = d["clear_color_b"].GetInt();

	int scale = 1;
	if (d.HasMember("global_scale") && d["global_scale"].IsInt())
		scale = d["global_scale"].GetInt();

	/* ----- Read scene information from file ----- */ 
	glm::ivec2 disp_res = { X_DEFAULT_RES, Y_DEFAULT_RES };

	string scene_path = SCENES_FOLDER_PATH + scene_name + ".json";
	//check that scene file exists
	if (!std::filesystem::exists(scene_path)) {
		cout << "error: " << scene_name << ".json missing";
		exit(0);
	}

	EngineUtils::ReadJsonFile(scene_path, d);

	//if d has no layers, exit early
	if (!(d.HasMember("layers") && d["layers"].IsArray())) {
		cout << "Scene file has no layers... exiting";
		exit(0);
	}


	/* ----- Create Renderer ----- */
	Renderer::init(d, title, clear_color, scale);

	/* ----- Create Audio DB ----- */
	AudioDB::init();

	/* ----- Create Scene DB ----- */
	SceneDB::init(d);

	is_running = true;
}

void Engine::input()
{
	SDL_Event e;
	while (Helper::SDL_PollEvent498(&e)) 
	{
		if (Input::ProcessEvent(e)) continue;

		if (e.type == SDL_QUIT) {
			is_running = false;
			continue;
		}
	}

}

// do not print in this function
void Engine::update()
{
	SceneDB::tick();

	Input::LateUpdate();
}

void Engine::render()
{
	Renderer::disp();
}


