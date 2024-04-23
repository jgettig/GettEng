#include "Transform.h"
#include "Renderer.h"

#include <iostream>
#include <sstream>
#include <string>

using std::cout;
using std::endl;
using std::string;

int Transform::read_json(const rapidjson::Value& obj)
{
	int idx = -1;
	if (obj.HasMember("properties") && obj["properties"].IsArray()) {
		for (int i = 0; i < obj["properties"].GetArray().Size(); ++i) {
			auto& prop = obj["properties"].GetArray()[i];
			if (prop.HasMember("name") && prop["name"].IsString() && prop["name"].GetString() == "transform") {
				idx = i;
				break;
			}
		}
	}

	const glm::vec2& ppu = Renderer::get_ppu();

	if (obj.HasMember("x") && obj["x"].IsNumber())
		position.x = obj["x"].GetFloat() / ppu.x;
	if (obj.HasMember("y") && obj["y"].IsNumber())
		position.y = obj["y"].GetFloat() / ppu.y;

	if (idx == -1) return -1;

	auto& properties = obj["properties"].GetArray()[idx];
	if (properties["value"].GetString() == "") return idx;

	std::istringstream prop_stream(properties["value"].GetString());

	string prop_line;
	while (std::getline(prop_stream, prop_line)) {
		std::istringstream line_stream(prop_line);
		string prop;
		string val;
		char junk;
		line_stream >> prop >> junk >> val;

		if (junk != '=') {
			cout << "Error: transform tried to read property from json but didn't encounter an equals sign (did you put a space before and after it?)";
			exit(0);
		}

		if (prop == "scale_x") {
			scale.x = std::stof(val);
		}
		else if (prop == "scale_y") {
			scale.y = std::stof(val);
		}
		else if (prop == "pivot_x") {
			pivot.x = std::stof(val);
		}
		else if (prop == "pivot_y") {
			pivot.y = std::stof(val);
		}
		else if (prop == "rotation") {
			rotation_deg = std::stof(val);
		}

	}

	return idx;
}
