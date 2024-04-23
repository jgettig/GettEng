#include "TextDB.h"
#include "Consts.h"

#include <iostream>
#include <filesystem>

using std::cout;
using std::endl;
using std::string;
using std::unordered_map;
using std::vector;

void TextDB::init (SDL_Renderer* _r) 
{
    if (initialized) {
        cout << "error: double TextDB init call";
        exit(0);
    }

    r = _r;

    text_cache = unordered_map<string, vector<text_info>>();
    fonts = unordered_map<string, unordered_map<uint16_t, TTF_Font*>>();

    TTF_Init();

    initialized = true;
}

void TextDB::deinit()
{
    if(!initialized) {
        cout << "error: tried to deinit an uninitialized TextDB";
        exit(0);
    }

    for (auto& f_pair : fonts) {
        for (auto& s_pair : f_pair.second) {
            TTF_CloseFont(s_pair.second);
        }
    }

    for (auto& e : text_cache) {
        for (text_info& i : e.second) {
            SDL_DestroyTexture(i.t);
        }
    }

    initialized = false;
}


SDL_Texture* TextDB::get_text(const std::string& font_name, const string& text, uint16_t size, SDL_Color font_color)
{
    check_init();

    TTF_Font* font = get_font(font_name, size);

    if (text_cache.count(text) != 0) {
        text_info* info = nullptr;
        for (text_info& i : text_cache[text]) {
            if (i.font == font &&  i.c.r == font_color.r && i.c.g == font_color.g && i.c.b == font_color.b && i.c.a == font_color.a) {
                info = &(i);
                break;
            }
        }

        if (info != nullptr) {
            return info->t;
        }
    }

    SDL_Surface* surf = TTF_RenderText_Solid(font, text.c_str(), font_color);

    if (surf == nullptr) {
        cout << "error: failed to create surface for text";
        exit(0);
    }

    SDL_Texture* ret = SDL_CreateTextureFromSurface(r, surf);

    if (text_cache.count(text) == 0) text_cache.emplace(text, vector<text_info>());
    text_info new_entry = { ret, font, font_color };
    text_cache[text].emplace_back(std::move(new_entry));

    return ret; 
}



/* ------------------------ private ------------------------ */


TTF_Font* TextDB::get_font(const std::string& font_name, uint16_t font_size)
{
    if (fonts.count(font_name) == 0 || fonts[font_name].count(font_size) == 0)
        return create_font(font_name, font_size);
    else
        return fonts[font_name][font_size];
}

TTF_Font* TextDB::create_font(const std::string& font_name, uint16_t font_size)
{
    string path = FONTS_FOLDER_PATH + font_name + ".ttf";

    if (!std::filesystem::exists(path)) {
        cout << "error: font " << font_name << " missing";
        exit(0);
    }

    TTF_Font* new_font = TTF_OpenFont(path.c_str(), static_cast<int>(font_size));

    if (new_font == nullptr) {
        cout << "error: could not load font";
        exit(0);
    }

    if (fonts.find(font_name) == fonts.end()) fonts.emplace(font_name, unordered_map<uint16_t, TTF_Font*>());
    fonts[font_name].emplace(font_size, new_font);

    return new_font;
}

void TextDB::check_init()
{
    if (!initialized) {
        cout << "error: called TextDB function before initializing";
        exit(0);
    }
}
