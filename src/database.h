#ifndef _MUTTICAL_DATABASE_H_
#define _MUTTICAL_DATABASE_H_

#include "timezone.h"
#include "event.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <vector>
#include <iostream>
#include <regex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <tuple>

class database {
	public:
		database(std::string);
		void convert_ics_to_vector(std::string &);
		void extract_timezones();
		void extract_all_events();
		std::vector<Timezone> zones; //a vector with all timezones
		std::vector<Event> events;
	protected:
		std::string filename; //name of the .ics file
		std::string document; //a string version of the .ics file
		std::vector<std::string>all_lines; //a vector which holds all lines which matter 
		std::vector<lineinfo> all_info; //a vector which holds all information of lines which matter
		std::regex key_reg_1;
		std::regex key_reg_2;
		std::regex key_reg_both;
		std::vector<std::string> keys_we_need; //helps us extract the relevant information
		void build_from(std::string &);
		void extract_time_from_event(Event &e);
		void extract_summary(Event &e);
		void extract_description(Event &e);
		void extract_organizer(Event &e);
		void extract_attendee(Event &e);
		void extract_timezone_for_event(std::vector<std::pair<int, int>> event_indices);
		lineinfo *find_line(Event &e, const std::string &key);
		lineinfo *find_line(const std::string &key);

};

#endif //_MUTTICAL_DATABASE_H_
