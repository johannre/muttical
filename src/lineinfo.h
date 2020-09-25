#ifndef __LINE_INFO__
#define __LINE_INFO__

#include <map>

struct lineinfo {
	std::string line;
	std::string key;
	std::string value;
	std::map<std::string, std::string> params;
	int index_in_all_lines;
};

#endif //__LINE_INFO__

