#ifndef TEMPLATE_DB_H
#define TEMPLATE_DB_H

#include <unordered_map>
#include <string>

#include "Actor.h"

class TemplateDB
{
public:

	static void init();
	static void deinit();

	//Constructs a template actor for template_name if one does not exist, and returns a copy of it
	static const Actor* get_template_actor(std::string template_name);

	static bool is_init() { return initialized; }

private:

	inline static std::unordered_map<std::string, Actor> templates;
	inline static bool initialized = false;

	static Actor* create_template(std::string template_name);

	static void check_init();
};

#endif