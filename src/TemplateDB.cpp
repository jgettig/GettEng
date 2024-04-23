#include "TemplateDB.h"

#include <filesystem>
#include <iostream>

#include "rapidjson/document.h"
#include "EngineUtils.h"
#include "Consts.h"

using std::cout;
using std::endl;

void TemplateDB::init()
{
	if (initialized) {
		cout << "error: double TemplateDB init call";
		exit(0);
	}

	initialized = true;
}

void TemplateDB::deinit()
{
	if (!initialized) {
		cout << "error: tried to deinit an uninitialized TemplateDB";
		exit(0);
	}

	initialized = false;
}

const Actor* TemplateDB::get_template_actor(std::string template_name)
{
	check_init();

	if (templates.count(template_name) == 0)
		return create_template(template_name);
	else
		return &templates[template_name];
}


/* ------------------------ private ------------------------ */


Actor* TemplateDB::create_template(std::string template_name)
{
	std::string path = TEMPLATES_FOLDER_PATH + template_name + ".template";

	if (!std::filesystem::exists(path)) {
		cout << "error: template " << template_name << " is missing";
		exit(0);
	}

	rapidjson::Document d;
	EngineUtils::ReadJsonFile(path, d);

	Actor* temp = nullptr;
	if (d.HasMember("template") && d["template"].IsString())
		temp = create_template(d["template"].GetString());

	templates.emplace(template_name, Actor(d, true));

	return &templates[template_name];
}

void TemplateDB::check_init()
{
	if (!initialized) {
		cout << "error: tried to deinit an uninitialized TemplateDB";
		exit(0);
	}
}
