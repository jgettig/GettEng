#ifndef TILEMAP_H
#define TILEMAP_H

#include "rapidjson/document.h"

#include <string>
#include <vector>

class Tilemap
{
public:
	Tilemap(const rapidjson::Value& layer, const std::string map_name);

	void draw();

private:
	std::vector<size_t> tiles_linear;
	std::string map_name;
	std::string sheet_name;
	size_t spacing_px = 0;
	size_t sheet_row_len = 0;
	size_t map_row_len = 0;
};

#endif