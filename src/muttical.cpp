#include "cmdline.h"
#include "database.h"
#include "output.h"

#include <iostream>
#include <filesystem>

//tells if something went wrong in the functions. If it's true, the main function
//returns -1
bool error = false; 

//this function checks if there are inconsitent inputs from the user. If so, the 
//program stops and the user gets an matching error-message. 
bool valid_inputs() {

	if (cmdline.force && !cmdline.save_events) {
		std::cout << "failed to overwrite " << cmdline.filename  << ": -f/--force only works with -s/--save" << std::endl; 
		return false;
	}
	if (cmdline.append && !cmdline.save_events) {
		std::cout << "failed to append to " << cmdline.filename  << ": -a/--append only works with -s/--save" << std::endl; 
		return false;
	}
	if (cmdline.force && cmdline.append) { 
		std::cout << "failed to force and append to " << cmdline.filename << ": inconsistent use of -a/--append and -f/--force" << std::endl;
		return false;
	}

	return true;

}

int main(int argc, char **argv) {

	try {
		parse_cmdline(argc, argv);
	} catch (...) {
		std::cerr << "Invalid Argument: -e requires a numeric argument" << std::endl;
		return -1;
	}

	if (!valid_inputs()) return -1;

	database x(cmdline.input_name);

	if (cmdline.interactive) interactive_mode(x);

	int event_index = cmdline.event_index; //the user will decide this number, that's just an interim solution until I know how to get
	//the number from the cmdline-parser

	//if the -e flag is entered, the other flags only apply to the event with the entered index
	if (cmdline.show_event_nr) {

		if (more_than_one(cmdline.show_events_list, cmdline.show_description_list, cmdline.show_participants, cmdline.save_events, cmdline.upcoming)) {
			bool error = false;
			print_more_than_one(x, event_index, error);
		}
		else if (cmdline.show_events_list) print_summary(x, event_index);
		else if (cmdline.show_description_list) print_description(x, event_index);
		else if (cmdline.show_participants) print_participants(x, event_index);
		else if (cmdline.save_events) {
			if (std::filesystem::exists(cmdline.filename)) {
				if (cmdline.force) 
					save_event_in_ics(x, event_index, cmdline.filename, true, true, false, true);
				else if (cmdline.append)
					save_event_in_ics(x, event_index, cmdline.filename, true, true, true, false);
				else std::cout << "failed to create " << cmdline.filename << ": file exists" << std::endl, error = true;
			} else 
				save_event_in_ics(x, event_index, cmdline.filename, true, true, false, false);
		} 
		else if (cmdline.upcoming) {
			print_near_event(x, event_index, false);
		}

	} else {

		if (cmdline.show_zone_list) print_zones(x);
		if (more_than_one(cmdline.show_events_list, cmdline.show_description_list, cmdline.show_participants, cmdline.save_events, cmdline.upcoming))
			print_more_than_one_all(x);
		else {
			if (cmdline.show_events_list) print_summaries(x);
			if (cmdline.show_description_list) print_descriptions(x);
			if (cmdline.show_participants) print_all_participants(x);
			if (cmdline.save_events) save_events_in_ics(x, "");
			if (cmdline.upcoming) print_near_events(x);
		}

	}

	//if in one of the called functions something went wrong, the main returns -1. 
	//Else it returns 0.
	if (error) return -1;
	else return 0;
}


