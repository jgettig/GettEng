#include "ComponentDB.h"
#include "Consts.h"
#include "Actor.h"
#include "Input.h"
#include "SceneDB.h"
#include "Renderer.h"
#include "AudioDB.h"
#include "Engine.h"
#include "Transform.h"

#include "Helper.h"
#include "keycode_to_scancode.h"

#include <filesystem>
#include <thread>
#include <cstdlib>
#include <sstream>
#include <string>

using std::string;
using std::cout;
using std::endl;

using luabridge::LuaRef;

void ComponentDB::init()
{
	if (initialized) {
		cout << "error: double ComponentDB init call";
		exit(0);
	}

	state = luaL_newstate();
	luaL_openlibs(state);

	/* add classes (such as Actor) to global state */
	add_global_classes();

	/* add namespaces and functions (such as Debug.Log) to global state */
	add_global_functions();

	/* read in all components */
	if (!std::filesystem::exists(COMPONENT_FOLDER_PATH)) return;

	for (const auto& entry : std::filesystem::directory_iterator(COMPONENT_FOLDER_PATH)) {

		std::string compname = entry.path().filename().stem().string();

		if (luaL_dofile(state, entry.path().string().c_str()) != LUA_OK) {
			cout << "problem with lua file " << compname;
			exit(0);
		}


		LuaRef new_comp_prototype = luabridge::getGlobal(state, compname.c_str());
	}
	
	initialized = true;
}

void ComponentDB::deinit()
{
	if (!initialized) {
		cout << "error: tried to deinit an uninitialized ComponentDB";
		exit(0);
	}

	lua_close(state);

	initialized = false;
}



int ComponentDB::get_component(LuaRef& instance_table, std::string comp_name)
{
	LuaRef parent_table = luabridge::getGlobal(state, comp_name.c_str());
	if (!parent_table.isTable()) {
		cout << "error: failed to locate component " << comp_name;
		exit(0);
	}
	return inherit(instance_table, parent_table);
}

int ComponentDB::read_component_json_A2(luabridge::LuaRef* component_table, const rapidjson::Value& comp_json)
{

	for (auto& property : comp_json.GetObject()) {
		std::string p_name = property.name.GetString();

		if (property.value.IsBool())
			(*component_table)[p_name] = property.value.GetBool();

		else if (property.value.IsString())
			(*component_table)[p_name] = property.value.GetString();

		else if (property.value.IsInt())
			(*component_table)[p_name] = property.value.GetInt();

		else if (property.value.IsDouble())
			(*component_table)[p_name] = property.value.GetDouble();

		else if (property.value.IsNull())
			(*component_table)[p_name] = luabridge::Nil();

		else {
			//unsupported type
			cout << "undefined type in actor declaration";
			exit(0);
		}

	}

	return 0;
}

int ComponentDB::read_component_json(luabridge::LuaRef* component_table, const rapidjson::Value& comp_json)
{

	std::istringstream prop_stream(comp_json["value"].GetString());

	string prop_line;
	while (std::getline(prop_stream, prop_line)) {
		std::istringstream line_stream(prop_line);
		string prop;
		string val;
		char junk;
		line_stream >> prop >> junk >> val;

		if (junk != '=') {
			cout << "Error: transform tried to read property from json but didn't encounter an equals sign (did you put a space before and after it?)";
			exit(0);
		}

		bool failed;
		float fval;
		try {
			failed = false;
			fval = stof(val);
		}
		catch (const std::exception& e) {
			failed = true;
		}
		if (!failed) {
			(*component_table)[prop] = fval;
			return 0;
		}
		
		float ival;
		try {
			failed = false;
			ival = stoi(val);
		}
		catch (const std::exception& e) {
			failed = true;
		}
		if (!failed) {
			(*component_table)[prop] = ival;
			return 0;
		}

		if (val == "true") (*component_table)[prop] = true;
		else if (val == "false") (*component_table)[prop] = false;
		else if (val == "null") (*component_table)[prop] = luabridge::Nil();

		(*component_table)[prop] = val;

	}
}

int ComponentDB::inherit(LuaRef& instance_table, const LuaRef& parent_table)
{
	/* create metatable */
	LuaRef new_metatable = luabridge::newTable(state);
	new_metatable["__index"] = parent_table;

	/* use c api to set metatable */
	instance_table.push(state);
	new_metatable.push(state);
	lua_setmetatable(state, -2);
	lua_pop(state, 1); // puts the stack back as it was

	return 0;
}

/* ------------------------ private ------------------------ */



void ComponentDB::add_global_classes()
{
	/*
	luabridge::getGlobalNamespace(state)
		.beginClass<[cpp class equiv]>("[lua name]")
		.addData("[prop]", [cpp class prop])
		.addFunction("[name]", &[cpp class method])
		.endClass();
	*/

	//glm vec2
	luabridge::getGlobalNamespace(state)
		.beginClass<glm::vec2>("vec2")
		.addData("x", &glm::vec2::x)
		.addData("y", &glm::vec2::y)
		.endClass();
	
	//Actor
	luabridge::getGlobalNamespace(state)
		.beginClass<Actor>("Actor")
		.addFunction("GetName", &Actor::get_name)
		.addFunction("GetID", &Actor::get_id)
		.addFunction("GetComponentByKey", &Actor::get_component_by_key)
		.addFunction("GetComponent", &Actor::get_component)
		.addFunction("GetComponents", &Actor::get_components)
		.addFunction("AddComponent", &Actor::cpp_add_component)
		.addFunction("RemoveComponent", &Actor::cpp_remove_component)
		.endClass();

	//Transform
	luabridge::getGlobalNamespace(state)
		.beginClass<Transform>("Transform")
		.addFunction("Translate", &Transform::translate)
		.addFunction("Rotate", &Transform::rotate)
		.addFunction("Scale", &Transform::rescale)
		.addData("position", &Transform::position)
		.addData("scale", &Transform::scale)
		.addData("rotation", &Transform::rotation_deg)
		.addData("pivot", &Transform::pivot)
		.endClass();
}
 
void ComponentDB::add_global_functions()
{
	/*
	luabridge::getGlobalNamespace(state)
		.beginNamespace("[Namespace]")
		.addData("[prop]", [cpp class prop])
		.addFunction("[name]", &[cpp class method])
		.endClass();
	*/

	//Actor global
	luabridge::getGlobalNamespace(state)
		.beginNamespace("Actor")
		.addFunction("Find", &Actor::get_actor)
		.addFunction("FindAll", &Actor::get_actors)
		.addFunction("Instantiate", &SceneDB::cpp_instantiate)
		.addFunction("Destroy", &SceneDB::cpp_destroy)
		.endNamespace();

	//Debug
	luabridge::getGlobalNamespace(state)
		.beginNamespace("Debug")
		.addFunction("Log", &LuaFuncs::cpp_log)
		.addFunction("LogError", &LuaFuncs::cpp_log_err)
		.endNamespace();

	//Application
	luabridge::getGlobalNamespace(state)
		.beginNamespace("Application")
		.addFunction("Quit", &LuaFuncs::cpp_quit)
		.addFunction("Sleep", &LuaFuncs::cpp_sleep)
		.addFunction("GetFrame", &LuaFuncs::cpp_getframe)
		.addFunction("OpenURL", &LuaFuncs::cpp_open)
		.endNamespace();

	//Input
	luabridge::getGlobalNamespace(state)
		.beginNamespace("Input")
		.addFunction("GetKey", &LuaFuncs::cpp_input_key)
		.addFunction("GetKeyDown", &LuaFuncs::cpp_input_keydown)
		.addFunction("GetKeyUp", &LuaFuncs::cpp_input_keyup)
		.addFunction("GetMousePosition", &Input::GetMousePosition)
		.addFunction("GetMouseButton", &Input::GetMouseButton)
		.addFunction("GetMouseButtonDown", &Input::GetMouseButtonDown)
		.addFunction("GetMouseButtonUp", &Input::GetMouseButtonUp)
		.addFunction("GetMouseScrollDelta", &Input::GetMouseScrollDelta)
		.endNamespace();

	luabridge::getGlobalNamespace(state)
		.beginNamespace("Text")
		.addFunction("Draw", &Renderer::draw_text)
		.endNamespace();

	luabridge::getGlobalNamespace(state)
		.beginNamespace("Audio")
		.addFunction("Play", &AudioDB::play_track)
		.addFunction("Halt", &AudioDB::stop_channel)
		.addFunction("SetVolume", &AudioDB::set_volume)
		.endNamespace();

	luabridge::getGlobalNamespace(state)
		.beginNamespace("Image")
		.addFunction("DrawUI", &Renderer::draw_UI)
		.addFunction("DrawUIEx", &Renderer::draw_UI_Ex)
		.addFunction("Draw", &Renderer::draw_sprite)
		.addFunction("DrawEx", &Renderer::draw_sprite_Ex)
		.addFunction("DrawPixel", &Renderer::draw_pixel)
		.endNamespace();

	luabridge::getGlobalNamespace(state)
		.beginNamespace("Camera")
		.addFunction("SetPosition", &Renderer::set_camera_pos)
		.addFunction("GetPositionX", &Renderer::get_camera_posX)
		.addFunction("GetPositionY", &Renderer::get_camera_posY)
		.addFunction("SetZoom", &Renderer::set_camera_zoom)
		.addFunction("GetZoom", &Renderer::get_camera_zoom)
		.endNamespace();

	luabridge::getGlobalNamespace(state)
		.beginNamespace("Scene")
		.addFunction("Load", &Engine::load_scene_cpp)
		.addFunction("GetCurrent", &Engine::get_scene_name)
		.addFunction("DontDestroy", &SceneDB::keep_actor)
		.endNamespace();
}

void ComponentDB::check_init()
{
	if (!initialized) {
		cout << "error: called ComponentDB function before initializing";
		exit(0);
	}
}

void LuaFuncs::cpp_sleep(int ms)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

int LuaFuncs::cpp_getframe()
{
	return Helper::GetFrameNumber();
}

void LuaFuncs::cpp_open(const std::string& url)
{
#ifdef _WIN32
	std::string cmd = "start " + url;
#elif __APPLE__
	std::string cmd = "open " + url;
#else
	std::string cmd = "xdg-open " + url;
#endif


	std::system(cmd.c_str());
}

bool LuaFuncs::cpp_input_key(const std::string& code)
{
	if (__keycode_to_scancode.find(code) == __keycode_to_scancode.end()) return false;
	return Input::GetKey(__keycode_to_scancode.at(code));
}

bool LuaFuncs::cpp_input_keydown(const std::string& code)
{
	if (__keycode_to_scancode.find(code) == __keycode_to_scancode.end()) return false;
	return Input::GetKeyDown(__keycode_to_scancode.at(code));
}

bool LuaFuncs::cpp_input_keyup(const std::string& code)
{
	if (__keycode_to_scancode.find(code) == __keycode_to_scancode.end()) return false;
	return Input::GetKeyUp(__keycode_to_scancode.at(code));
}
