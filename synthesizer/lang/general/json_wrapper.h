#include <memory>

/* it is assumed that each GJson is a single k-v pair/a string/an array */
class GJson {
	public:
	GJson();
	GJson(std::string json_src);
	bool is_array();
	int size(); /* only array has size */
	std::shared_ptr<GJson> get(int index); /* if not array, only 0 is available, which is same as value() */
	std::string name(); /* array has no name, string has only name */
	std::shared_ptr<GJson> value(); /* string has no value, array's value can only be accessed by get() or [] */
};
