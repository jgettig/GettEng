#ifndef RENDERER_H
#define RENDERER_H
#include <string>
#include <vector>
#include <optional>
#include <deque>

#include "SDL2/SDL.h"
#include "glm/glm.hpp"

#include "ImageDB.h"
#include "TextDB.h"
#include "Actor.h"
#include "Consts.h"


class Renderer
{
public:

	static void init(
		int res_x, int res_y, std::string title = "", 
		SDL_Color clearColor = { 255, 255, 255, 255 }
		);

	static void discard_frame();
	static void disp();

	static void draw_sprite(const std::string& img_name, float x, float y);
	static void draw_sprite_Ex(const std::string& img_name, float x, float y, float rotation_degrees, float scale_x, float scale_y,
							   float pivot_x, float pivot_y, float r, float g, float b, float a, float sorting_order);

	static void draw_UI(const std::string& img_name, float x, float y);
	static void draw_UI_Ex(const std::string& img_name, float x, float y, float r, float g, float b, float a, float sorting_order);
	
	static void draw_text(const std::string& text, float x, float y, const std::string& font_name, float font_size, float r, float g, float b, float a);
	
	static void draw_pixel(float x, float y, float r, float g, float b, float a);

	static void set_camera_pos(float x, float y);
	static float get_camera_posX() { return cam_pos.x; }
	static float get_camera_posY() { return cam_pos.y; }
	static void set_camera_zoom(float factor);
	static float get_camera_zoom() { return scale; }

	static glm::ivec2 get_image_size(const std::string& img_name);

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
	};

	struct sprite_params : params{
		std::string img_name;
		float x = 0;
		float y = 0;
		int rot_deg = 0;
		float scale_x = 1.f;
		float scale_y = 1.f;
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

	static inline const uint32_t windowFlags = SDL_WINDOW_SHOWN;
	static inline const uint32_t renderFlags = SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED;
	static inline const uint32_t ppm = 100;

	static inline SDL_Window* window = nullptr;
	static inline SDL_Renderer* r = nullptr;

	static inline std::deque<sprite_params> spriteQueue;
	static inline std::deque<UI_params> UIQueue;
	static inline std::deque<text_params> textQueue;
	static inline std::deque<px_params> pxQueue;

	static inline SDL_Color clearColor;							//window clear color
	static inline glm::ivec2 cam_dims;							//window resolution
	static inline glm::vec2 cam_pos;

	static inline int max_id = 0;								//max request id (for debugging)
	static inline float scale = 1.0;							//current render scale (camera zoom)

	static inline bool initialized = false;

	static void clear();

	static void check_init();

	static void disp_sprites();
	static void disp_UI();
	static void disp_text();
	static void disp_px();
};

#endif