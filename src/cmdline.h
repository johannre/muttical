#pragma once

#include <string>

struct cmdline {
	std::string input_name;
	bool verbose;
	int event_index;
	cmdline() : verbose(false) {}
	//those variables represent the entered options
	bool show_zone_list = false;
	bool show_events_list = false;
	bool show_event_nr = false;
	bool got_event_index = false;
	bool show_description_list = false;
	bool show_participants = false;
};

extern cmdline cmdline;
int parse_cmdline(int argc, char **argv);
