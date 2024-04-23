#include "ImageDB.h"

#include <filesystem>
#include <iostream>

#include "Consts.h"

using std::cout;
using std::endl;
using std::string;

void ImageDB::init(SDL_Renderer* _r) {
	if (initialized) {
		cout << "error: double ImageDB init call";
		exit(0);
	}

	r = _r;

	initialized = true;
}

void ImageDB::deinit()
{
	if (!initialized) {
		cout << "error: tried to deinit an uninitialized ImageDB";
		exit(0);
	}

	for (std::pair<string, SDL_Texture*> t : images) {
		SDL_DestroyTexture(t.second);
	}

	initialized = false;
}

SDL_Texture* ImageDB::get_image(const string& name)
{
	if (images.count(name) == 0)
		return create_image(name);
	else
		return images[name];
}

/* ------------------------ private ------------------------ */

SDL_Texture* ImageDB::create_image(const string& name)
{
	string path = IMAGES_FOLDER_PATH + name + ".png";

	if (!std::filesystem::exists(path)) {
		cout << "error: missing image " << name;
		exit(0);
	}

	images.emplace(name, IMG_LoadTexture(r, path.c_str()));

	return images[name];
}

void ImageDB::check_init()
{
	if (!initialized) {
		cout << "error: called ImageDB function before initializing";
		exit(0);
	}
}
