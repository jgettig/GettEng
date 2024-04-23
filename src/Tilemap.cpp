#include "Tilemap.h"

#include "Consts.h"
#include "Renderer.h"
#include "EngineUtils.h"

#include <iostream>
#include <filesystem>

using std::cout;
using std::endl;
using std::string;

Tilemap::Tilemap(const rapidjson::Value& layer, const string map_name)
{
	this->map_name = map_name;

	if (!(layer.HasMember("data") && layer["data"].IsArray())) {
		cout << "error - tried to read in a tilemap with invalid scene data";
		exit(0);
	}

	const auto& data = layer["data"].GetArray();
	tiles_linear.reserve(data.Size());
	for (int i = 0; i < data.Size(); ++i) {
		tiles_linear.push_back(data[i].GetInt());
	}

	if (layer.HasMember("width") && layer["width"].IsInt())
		map_row_len = layer["width"].GetInt();

	rapidjson::Document d;
	if (!std::filesystem::exists(TILEMAP_FOLDER_PATH + map_name)) {
		cout << "error - missing tilemap file " << map_name << ". Expected in directory " << TILEMAP_FOLDER_PATH;
		exit(0);
	}
	EngineUtils::ReadJsonFile(TILEMAP_FOLDER_PATH + map_name, d);

	if (!(d.HasMember("image") && d["image"].IsString())) {
		cout << "error"; //TODO: make verbose
		exit(0);
	}
	
	sheet_name = std::filesystem::path(d["image"].GetString()).filename().stem().string();

	if (d.HasMember("columns") && d["columns"].IsInt())
		sheet_row_len = d["columns"].GetInt();

	if (d.HasMember("spacing") && d["spacing"].IsInt())
		spacing_px = d["spacing"].GetInt();

	if (sheet_row_len == 0 || map_row_len == 0) {
		cout << "error - read a tilesheet or map row length of 0 sprites";
		exit(0);
	}
}

void Tilemap::draw()
{
	for (size_t i = 0; i < tiles_linear.size(); ++i) {
		size_t map_row = i / map_row_len;
		size_t map_col = i % map_row_len;

		size_t tile_idx = tiles_linear[i] - 1;

		size_t sheet_row = tile_idx / sheet_row_len;
		size_t sheet_col = tile_idx % sheet_row_len;

		Renderer::draw_tile_sprite(sheet_name, sheet_row, sheet_col, map_row, map_col, spacing_px);
	}
}
