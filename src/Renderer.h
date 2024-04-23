#ifndef RENDERER_H
#define RENDERER_H
#include <string>
#include <vector>
#include <optional>
#include <deque>

#include "rapidjson/document.h"
#include "SDL2/SDL.h"
#include "glm/glm.hpp"

#include "ImageDB.h"
#include "TextDB.h"
#include "Actor.h"
#include "Consts.h"


class Renderer
{
public:

	static void init(rapidjson::Document& d, const std::string& title, const SDL_Color& clear_color = WHITE, int global_scale = 1);

	static void discard_frame();
	static void disp();

	static void draw_sprite(const std::string& img_name, float x, float y);
	static void draw_sprite_Ex(const std::string& img_name, float x, float y, float rotation_degrees, float scale_x, float scale_y,
							   float pivot_x, float pivot_y, float r, float g, float b, float a, float sorting_order);

	static void draw_UI(const std::string& img_name, float x, float y);
	static void draw_UI_Ex(const std::string& img_name, float x, float y, float r, float g, float b, float a, float sorting_order);
	
	static void draw_text(const std::string& text, float x, float y, const std::string& font_name, float font_size, float r, float g, float b, float a);
	
	static void draw_pixel(float x, float y, float r, float g, float b, float a);

	static void draw_tile_sprite(const std::string img_name, size_t src_row, size_t src_col, size_t dest_row, size_t dest_col, size_t px_offset);

	static void set_camera_pos(float x, float y);
	static float get_camera_posX() { return cam_pos.x; }
	static float get_camera_posY() { return cam_pos.y; }
	static void set_camera_zoom(float factor);
	static float get_camera_zoom() { return scale; }

	static glm::ivec2 get_image_size(const std::string& img_name);

	static const glm::vec2& get_ppu() { return ppu; }

#ifdef DEBUG
	static void draw_rect(SDL_Rect r, const SDL_Color c);
#endif

	static void deinit();

	//avoid use if possible
	static SDL_Renderer* get_renderer() { return r; }

	static bool is_init() { return initialized; }

private:

	struct params {
		int order = 0;
		uint8_t r = 255;
		uint8_t g = 255;
		uint8_t b = 255;
		uint8_t a = 255;
		float scale_x = global_scale;
		float scale_y = global_scale;
	};

	struct tile_params : params {
		std::string img_name;
		float x = 0;
		float y = 0;
		float sheet_x = 0;
		float sheet_y = 0;
		float pivot_x = 0.5f;
		float pivot_y = 0.5f;
	};

	struct sprite_params : params{
		std::string img_name;
		float x = 0;
		float y = 0;
		int rot_deg = 0;
		float pivot_x = 0.5f;
		float pivot_y = 0.5f;
	};

	struct UI_params : params {
		std::string img_name;
		int x = 0;
		int y = 0;
	};

	struct text_params : params {
		std::string content;
		std::string font;
		int x = 0;
		int y = 0;
		uint16_t font_size = 0;
	};

	struct px_params : params {
		int x = 0;
		int y = 0;
	};

	class params_compare {
	public:
		bool operator()(const params& lhs, const params& rhs) {
			return lhs.order < rhs.order;
		}
	};

	static inline params_compare comp;

#ifdef DEBUG
	struct RectParams {
		SDL_Rect rect;
		SDL_Color color;
	};
	std::vector<RectParams> rectQueue;
#endif

	//SDL pointers
	static inline SDL_Window* window = nullptr;
	static inline SDL_Renderer* r = nullptr;

	//render queues
	static inline std::deque<tile_params> tileQueue;
	static inline std::deque<sprite_params> spriteQueue;
	static inline std::deque<UI_params> UIQueue;
	static inline std::deque<text_params> textQueue;
	static inline std::deque<px_params> pxQueue;

	//rendering info
	static inline SDL_Color clearColor;										//window clear color
	static inline glm::vec2 ppu = { 1, 1 };									//screen pixels per world unit
	static inline glm::vec2 res = { X_DEFAULT_RES, Y_DEFAULT_RES };			//window size in pixels
	static inline int global_scale;
	static inline glm::vec2 scaled_ppu;

	//camera info
	static inline glm::vec2 cam_dims = { X_DEFAULT_RES, Y_DEFAULT_RES };	//window resolution in world units
	static inline glm::vec2 cam_pos = { 0.f, 0.f };							//camera position in world units
	static inline float scale = 1.0;										//current render scale (camera zoom)

	//debug info
	static inline int max_id = 0;											//max request id (for debugging)

	//static class initialized field
	static inline bool initialized = false;

	static void clear();

	static void check_init();

	static void disp_tiles();
	static void disp_sprites();
	static void disp_UI();
	static void disp_text();
	static void disp_px();

	static void read_camera_json(rapidjson::Document& d);
};

#endif