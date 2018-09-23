#include "json_wrapper.h"
#include <iostream>

using std::cout;
using std::endl;

GJson::GJson() {
}

GJson::GJson(string json_src) {
	string err = "";
	real_json = Json::parse(json_src, err);
	if (err != "")
		cout << err << endl;
}

GJson::GJson(Json real_json) {
	this->real_json = real_json;
}

bool GJson::is_array() {
	return real_json.is_array();
}

int GJson::size() {
	if (is_array())
		return real_json.array_items().size();
	else
		return 1;
}

std::shared_ptr<GJson> GJson::get(int index) {
	return shared_ptr<GJson>(new GJson(real_json[index]));
}

std::string GJson::name() {
	if (real_json.is_string())
		return real_json.string_value();
	else
		return real_json.object_items().begin()->first;
}

std::shared_ptr<GJson> GJson::value() {
	return shared_ptr<GJson>(new GJson(real_json.object_items().begin()->second));
}
