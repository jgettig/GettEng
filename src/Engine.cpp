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
#include <glm/glm.hpp>
#include <rapidjson/document.h>

//course headers
#include "Helper.h"

//internal headers
#include "Actor.h"
#include "EngineUtils.h"
#include "Consts.h"
#include "Input.h"
#include "ComponentDB.h"

#include "Lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"


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
			SceneDB::load_scene(next_scene);
			scene_name = next_scene;
			next_scene.clear();

			do_sceneChange = false;
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

void Engine::load_scene(const std::string& new_scene)
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
		next_scene = d["initial_scene"].GetString();

	if (next_scene == "") {
		cout << "error: initial_scene unspecified";
		exit(0);
	}

	/* ----- Read rendering config from file ----- */ 
	glm::ivec2 disp_res = { X_DEFAULT_RES, Y_DEFAULT_RES };

	SDL_Color clear_color = { 255, 255, 255, 255 };
	float zoom = 1;
	bool flip = false;

	//check for resources/rendering.config
	if (std::filesystem::exists(std::filesystem::path(RENDERING_CONFIG_PATH))) {
		//read file
		EngineUtils::ReadJsonFile(RENDERING_CONFIG_PATH, d);

		if (d.HasMember("x_resolution") && d["x_resolution"].IsInt())
			disp_res.x = d["x_resolution"].GetInt();
		if (d.HasMember("y_resolution") && d["y_resolution"].IsInt())
			disp_res.y = d["y_resolution"].GetInt();

		/*if (d.HasMember("cam_offset_x") && d["cam_offset_x"].IsNumber())
			cam_offset.x = d["cam_offset_x"].GetFloat();
		if (d.HasMember("cam_offset_y") && d["cam_offset_y"].IsNumber())
			cam_offset.y = d["cam_offset_y"].GetFloat();*/

		if (d.HasMember("clear_color_r") && d["clear_color_r"].IsInt())
			clear_color.r = d["clear_color_r"].GetInt();
		if (d.HasMember("clear_color_g") && d["clear_color_g"].IsInt())
			clear_color.g = d["clear_color_g"].GetInt();
		if (d.HasMember("clear_color_b") && d["clear_color_b"].IsInt())
			clear_color.b = d["clear_color_b"].GetInt();

		if (d.HasMember("zoom_factor") && d["zoom_factor"].IsNumber())
			zoom = d["zoom_factor"].GetFloat();

		/*if (d.HasMember("cam_ease_factor") && d["cam_ease_factor"].IsNumber())
			camera_ease = d["cam_ease_factor"].GetFloat();*/
	}
	 
	/* ----- Create Renderer ----- */
	Renderer::init(disp_res.x, disp_res.y, title, clear_color);
	Renderer::set_camera_zoom(zoom);

	/* ----- Create Audio DB ----- */
	AudioDB::init();

	/* ----- Create Scene DB ----- */
	SceneDB::init();

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


