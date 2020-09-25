#ifndef _MUTTICAL_TIMEZONE_
#define _MUTTICAL_TIMEZONE_

#include <string>

class Timezone {
	public:
		Timezone() : timezone_infos("") { };
		Timezone(std::string timezone_infos) : my_tzid(timezone_infos) { };
		std::string &timezone() { return timezone_infos; }
		const std::string timezone() const { return timezone_infos; }
		std::string &tzid() { return my_tzid; }
		const std::string tzid() const { return my_tzid; }
		int index; //index in all_lines
	private:
		std::string timezone_infos;
		std::string my_tzid;
};

#endif //_MUTTICAL_TIMEZONE_
