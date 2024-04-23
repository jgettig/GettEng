#ifndef CONSTS_H
#define CONSTS_H

#include <string>
#include "SDL2/SDL.h"

//#define DEBUG true

inline const std::string RESOURCES_PATH = "resources";
inline const std::string GAME_CONFIG_PATH = RESOURCES_PATH + "/game.config";
//inline const std::string RENDERING_CONFIG_PATH = RESOURCES_PATH + "/rendering.config";
inline const std::string SCENES_FOLDER_PATH = RESOURCES_PATH + "/scenes/";
inline const std::string TEMPLATES_FOLDER_PATH = RESOURCES_PATH + "/actor_templates/";
inline const std::string IMAGES_FOLDER_PATH = RESOURCES_PATH + "/images/";
inline const std::string FONTS_FOLDER_PATH = RESOURCES_PATH + "/fonts/";
inline const std::string AUDIO_FOLDER_PATH = RESOURCES_PATH + "/audio/";
inline const std::string COMPONENT_FOLDER_PATH = RESOURCES_PATH + "/component_types/";
inline const std::string TILEMAP_FOLDER_PATH = RESOURCES_PATH + "/tilesets/";

inline const std::string ENGINE_RES_PATH = "resources_internal";
inline const std::string ENGINE_COMPONENT_FOLDER_PATH = ENGINE_RES_PATH + "/component_types/";


inline const std::string LAYER_CAMERA_NAME = "Layer_Camera";
inline const std::string LAYER_OBJECTS_NAME = "GameObjects";
inline const std::string LAYER_MAP_NAME = "TileMap";

inline const SDL_Color WHITE = { 255, 255, 255, 255 };
inline const SDL_Color RED = { 255, 0, 0, 255 };
inline const SDL_Color BLUE = { 0, 0, 255, 255 };

inline const int X_DEFAULT_RES = 640;
inline const int Y_DEFAULT_RES = 360;

#endif