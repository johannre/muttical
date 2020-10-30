#pragma once

#include <string>

struct cmdline {
	std::string input_name;
	bool verbose;
	int event_index;
	cmdline() : verbose(false), event_index(0) {}
	//those variables represent the entered options
	bool show_zone_list = false;
	bool show_events_list = false;
	bool show_event_nr = false;
	bool got_event_index = false;
	bool show_description_list = false;
	bool show_participants = false;
	bool save_events = false;
	std::string filename;
	bool force = false;
	bool append = false;
	bool upcoming = false;
	bool interactive = false;
};

extern cmdline cmdline;
int parse_cmdline(int argc, char **argv);
