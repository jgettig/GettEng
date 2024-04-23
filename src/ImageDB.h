#ifndef IMAGE_DB_H
#define IMAGE_DB_H
#include <string>
#include <unordered_map>

#include "SDL2/SDL.h"
#include "SDL_image/SDL_image.h"

class ImageDB
{
public:

	static void init(SDL_Renderer* _r);
	static void deinit();

	static SDL_Texture* get_image(const std::string& name);


	static bool is_init() { return initialized; }

private:


	static inline std::unordered_map<std::string, SDL_Texture*> images;
	static inline SDL_Renderer* r = nullptr;
	static inline bool initialized = false;

	static SDL_Texture* create_image(const std::string& name);

	static void check_init();
};

#endif