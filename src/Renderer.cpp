#include "Renderer.h"
#include "Helper.h"
#include "Consts.h"

#include "SDL_image/SDL_image.h"

#include <iostream>
#include <cmath>
#include <algorithm>

using std::cout;
using std::endl;
using std::string;

void Renderer::init(int res_x, int res_y, string title, 
				   SDL_Color clearColor)
{
	if (initialized) {
		cout << "error: double Renderer init call";
		exit(0);
	}

	window = Helper::SDL_CreateWindow498(title.c_str(), -800, 500, res_x, res_y, windowFlags);
	r = Helper::SDL_CreateRenderer498(window, -1, renderFlags);

	if (!TextDB::is_init()) {
		TextDB::init(r);
	}

	if (!ImageDB::is_init()) {
		ImageDB::init(r);
	}

	Renderer::clearColor = clearColor;
	cam_dims = { res_x, res_y };
	cam_pos = { 0, 0 };

	clear();
	initialized = true;
}

void Renderer::deinit()
{
	if (!initialized) {
		cout << "error: tried to deinit an uninitialized Renderer";
		exit(0);
	}

	SDL_DestroyRenderer(r);
	SDL_DestroyWindow(window);

	initialized = false;
}

void Renderer::disp()
{
	check_init();

	//clear last frame
	clear();

	//render sprites -> UI -> text -> pixels 
	disp_sprites();
	disp_UI();
	disp_text();
	disp_px();

	Helper::SDL_RenderPresent498(r);

	max_id = 0;
}

void Renderer::draw_sprite(const std::string& img_name, float x, float y)
{
	sprite_params new_req;

	new_req.img_name = img_name;
	new_req.x = x;
	new_req.y = y;

	spriteQueue.push_back(new_req);
}

void Renderer::draw_sprite_Ex(const std::string& img_name, float x, float y, float rotation_degrees, float scale_x, float scale_y, 
							  float pivot_x, float pivot_y, float r, float g, float b, float a, float sorting_order)
{
	sprite_params new_req;

	new_req.img_name = img_name;
	new_req.x = x;
	new_req.y = y;
	new_req.scale_x = scale_x;
	new_req.scale_y = scale_y;
	new_req.pivot_x = pivot_x;
	new_req.pivot_y = pivot_y;
	new_req.r = static_cast<uint8_t>(r);
	new_req.g = static_cast<uint8_t>(g);
	new_req.b = static_cast<uint8_t>(b);
	new_req.a = static_cast<uint8_t>(a);
	new_req.rot_deg = static_cast<int>(rotation_degrees);
	new_req.order = static_cast<int>(sorting_order);

	spriteQueue.push_back(new_req);
}

void Renderer::draw_UI(const std::string& img_name, float x, float y)
{
	UI_params new_req;

	new_req.img_name = img_name;
	new_req.x = static_cast<int>(x);
	new_req.y = static_cast<int>(y);

	UIQueue.push_back(new_req);
}

void Renderer::draw_UI_Ex(const std::string& img_name, float x, float y, float r, float g, float b, float a, float sorting_order)
{
	UI_params new_req;

	new_req.img_name = img_name;
	new_req.x = static_cast<int>(x);
	new_req.y = static_cast<int>(y);
	new_req.r = static_cast<uint8_t>(r);
	new_req.g = static_cast<uint8_t>(g);
	new_req.b = static_cast<uint8_t>(b);
	new_req.a = static_cast<uint8_t>(a);
	new_req.order = static_cast<int>(sorting_order);

	UIQueue.push_back(new_req);
}

void Renderer::draw_text(const std::string& text, float x, float y, const std::string& font_name, float font_size, float r, float g, float b, float a)
{
	text_params new_req;

	new_req.content = text;
	new_req.font = font_name;
	new_req.x = static_cast<int>(x);
	new_req.y = static_cast<int>(y);
	new_req.r = static_cast<uint8_t>(r);
	new_req.g = static_cast<uint8_t>(g);
	new_req.b = static_cast<uint8_t>(b);
	new_req.a = static_cast<uint8_t>(a);
	new_req.font_size = static_cast<uint16_t>(font_size);

	textQueue.push_back(new_req);
}

void Renderer::draw_pixel(float x, float y, float r, float g, float b, float a)
{
	px_params new_req;

	new_req.x = static_cast<int>(x);
	new_req.y = static_cast<int>(y);
	new_req.r = static_cast<uint8_t>(r);
	new_req.g = static_cast<uint8_t>(g);
	new_req.b = static_cast<uint8_t>(b);
	new_req.a = static_cast<uint8_t>(a);

	pxQueue.push_back(new_req);
}

void Renderer::set_camera_pos(float x, float y)
{
	cam_pos = glm::vec2(x, y);
}



void Renderer::discard_frame()
{
	check_init();

	spriteQueue.clear();
	UIQueue.clear();
	textQueue.clear();
	pxQueue.clear();
}

void Renderer::set_camera_zoom(float factor)
{
	check_init();

	scale = factor;
}

glm::ivec2 Renderer::get_image_size(const std::string& name)
{
	check_init();

	if (name == "") return glm::ivec2(0, 0);
	SDL_Texture* tex = ImageDB::get_image(name);
	glm::ivec2 size = { 0, 0 };
	SDL_QueryTexture(tex, NULL, NULL, &size.x, &size.y);
	return size;
}

#ifdef DEBUG
void Renderer::draw_rect(SDL_Rect rct, const SDL_Color c)
{
	check_init();

	RectParams rp = { rct, c };
	rectQueue.emplace_back(rp);
}
#endif



/* ------------------------ private ------------------------ */


void Renderer::clear()
{
	SDL_SetRenderDrawColor(r, clearColor.r, clearColor.g, clearColor.b, 255);
	SDL_RenderClear(r);
}


void Renderer::check_init()
{
	if (!initialized) {
		cout << "error: called Renderer function before initializing";
		exit(0);
	}
}

void Renderer::disp_sprites()
{
	std::stable_sort(spriteQueue.begin(), spriteQueue.end(), comp);

	SDL_RenderSetScale(r, scale, scale);

	while (!spriteQueue.empty()) {

		sprite_params req = spriteQueue.front();
		spriteQueue.pop_front();

		glm::vec2 cam_world_pos = cam_pos;
		glm::vec2 render_pos = glm::vec2(req.x, req.y) - cam_world_pos;

		SDL_Texture* tex = ImageDB::get_image(req.img_name);
		SDL_Rect tex_rect;
		SDL_QueryTexture(tex, NULL, NULL, &tex_rect.w, &tex_rect.h);

		//scale
		int flip = SDL_FLIP_NONE;
		if (req.scale_x < 0) flip |= SDL_FLIP_HORIZONTAL;
		if (req.scale_y < 0) flip |= SDL_FLIP_VERTICAL;

		float x_sc = std::abs(req.scale_x);
		float y_sc = std::abs(req.scale_y);
		
		tex_rect.w *= x_sc;
		tex_rect.h *= y_sc;

		SDL_Point pivot = { static_cast<int>(req.pivot_x * tex_rect.w), static_cast<int>(req.pivot_y * tex_rect.h) };

		tex_rect.x = static_cast<int>(ppm * render_pos.x + cam_dims.x * 0.5f * (1.0f / scale) - pivot.x);
		tex_rect.y = static_cast<int>(ppm * render_pos.y + cam_dims.y * 0.5f * (1.0f / scale) - pivot.y);

		SDL_SetTextureColorMod(tex, req.r, req.g, req.b);
		SDL_SetTextureAlphaMod(tex, req.a);

		Helper::SDL_RenderCopyEx498(0, "", r, tex, NULL, &tex_rect, req.rot_deg, &pivot, static_cast<SDL_RendererFlip>(flip));

		SDL_SetTextureColorMod(tex, 255, 255, 255);
		SDL_SetTextureAlphaMod(tex, 255);
	}

	SDL_RenderSetScale(r, 1, 1);
}

void Renderer::disp_UI()
{
	std::stable_sort(UIQueue.begin(), UIQueue.end(), comp);

	while (!UIQueue.empty()) {

		UI_params req = UIQueue.front();
		UIQueue.pop_front();

		SDL_Texture* img = ImageDB::get_image(req.img_name);

		int w = 0, h = 0;
		SDL_QueryTexture(img, NULL, NULL, &w, &h);

		SDL_Rect dst = { req.x, req.y, w, h };

		SDL_SetTextureColorMod(img, req.r, req.g, req.b);
		SDL_SetTextureAlphaMod(img, req.a);

		SDL_RenderCopy(r, img, NULL, &dst);

		SDL_SetTextureColorMod(img, 255, 255, 255);
		SDL_SetTextureAlphaMod(img, 255);
	}
}

void Renderer::disp_text()
{
	while (!textQueue.empty()) {

		text_params req = textQueue.front();
		textQueue.pop_front();

		SDL_Color c = {req.r, req.g, req.b, req.a };
		SDL_Texture* tex = TextDB::get_text(req.font, req.content, req.font_size, c);

		int w = 0, h = 0;
		SDL_QueryTexture(tex, NULL, NULL, &w, &h);

		SDL_Rect dst = { req.x, req.y, w, h };

		SDL_RenderCopy(r, tex, NULL, &dst);
	}
}

void Renderer::disp_px()
{
	SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);

	while (!pxQueue.empty()) {

		px_params req = pxQueue.front();
		pxQueue.pop_front();
		SDL_SetRenderDrawColor(r, req.r, req.g, req.b, req.a);

		SDL_RenderDrawPoint(r, req.x, req.y);
	}

	SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
}
