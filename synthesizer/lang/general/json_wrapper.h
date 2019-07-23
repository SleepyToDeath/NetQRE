#include <memory>
#include <string>
#include "json11/json11.hpp"
#include "rubify.hpp"

using std::shared_ptr;
using json11::Json;
using Rubify::string;

/* it is assumed that each GJson is a single k-v pair/a string/an array */
class GJson {
	public:
	GJson();
	GJson(string json_src);
	bool is_array();
	int size(); /* only array has size */
	std::shared_ptr<GJson> get(int index); /* if not array, only 0 is available, which is same as value() */
	std::string name(); /* array has no name, string has only name */
	std::shared_ptr<GJson> value(); /* string has no value, array's value can only be accessed by get() or [] */

	private:
	GJson(Json real_json);
	Json real_json;
};
