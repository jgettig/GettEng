#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "glm/glm.hpp"
#include "rapidjson/document.h"

class Transform
{
public:

	Transform() {}

	void translate(float dx, float dy) { position += glm::vec2(dx, dy); }
	void rotate(float deg) { rotation_deg += deg; }
	void rescale(float px, float py) { scale *= glm::vec2(px, py); }

	//takes obj, the json representing the object, and idx, the idx of the "transform" property
	int read_json(const rapidjson::Value& obj);

	glm::vec2 position = { 0.f, 0.f };
	glm::vec2 scale = { 1.f, 1.f };
	float rotation_deg = 0.f;
	glm::vec2 pivot = { 0.5f, 0.5f };

private:

};

#endif