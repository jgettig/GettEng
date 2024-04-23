#ifndef TEXT_DB_H
#define TEXT_DB_H

#include <string>
#include <unordered_map>
#include <vector>

#include "SDL2/SDL.h"
#include "SDL_ttf/SDL_ttf.h"

class TextDB
{
public:

	static void init(SDL_Renderer* r);

	static SDL_Texture* get_text(const std::string& font, const std::string& text, uint16_t size, SDL_Color font_color);

	static void deinit();

	static bool is_init() { return initialized; }

private:

	struct text_info {
		SDL_Texture* t;
		TTF_Font* font;
		SDL_Color c;
	};

	inline static std::unordered_map<std::string, std::unordered_map<uint16_t, TTF_Font*>> fonts; //caches fonts at sizes
	inline static std::unordered_map<std::string, std::vector<text_info>> text_cache; //caches text, but culls it when unused for a frame
	inline static SDL_Renderer* r = nullptr;
	inline static bool initialized = false;


	static TTF_Font* get_font(const std::string& font_name, uint16_t font_size);

	static TTF_Font* create_font(const std::string& font_name, uint16_t font_size);

	static void check_init();

};

#endif