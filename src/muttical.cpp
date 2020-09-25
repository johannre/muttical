#include "cmdline.h"
#include "database.h"

#include <cmath>
#include <iostream>
#include <iomanip>

//if the year of the end-event-date changed, we have to print the whole date again.
//To avoid indent-problems in the output, we have to know what kind of end-date 
//was printed. 
bool whole_date_print(const database &x) {
	for (const auto &i : x.events) {
		const tm &d = i.dtstart;
		const tm &e= i.dtend;
		if (d.tm_year != e.tm_year || d.tm_mon != e.tm_mon || d.tm_mday != e.tm_mday) 
			return true;
	}
	return false;
}

//returns a string with matching tabs inserted at newlines
std::string format_string(const std::string &str, int num_tabs) {
	std::string new_str = str;
	for (int i = 0; i < new_str.size(); i++) {
		if (new_str[i] == '\n') {
			for (int j = 0; j < num_tabs; j++) new_str.insert(i + 1, "\t");
		}
	}
	return new_str;
}

void print_date_and_time(const database &x, const std::tm &d, const std::tm &e) {
	if (d.tm_year == 0 && d.tm_mon == 0 && d.tm_mday == 0 && d.tm_hour == 0 && d.tm_min == 0) {
		std::cout << "No time info ...";
		return;
	}
	else {
		char date_and_time[32] = { 0 };
		strftime(date_and_time, 32, "%Y-%m-%d %H:%M", &d);
		std::cout << date_and_time;
	}
	if (e.tm_year == 0 && e.tm_mon == 0 && e.tm_mday == 0 && e.tm_hour == 0 && e.tm_min == 0)
		return;
	else {
		//if the year, month and day don't change, we only print hour:min
		if (d.tm_year == e.tm_year && d.tm_mon == e.tm_mon && d.tm_mday == e.tm_mday) {
			std::cout << " to ";
			if (whole_date_print(x)) std::cout << "Same day...";
			std::cout << std::setw(2) << std::setfill('0') << e.tm_hour << ":" << std::setw(2) << e.tm_min;
		}
		else {
			char date_and_time[32] = { 0 };
			strftime(date_and_time, 32, "%Y-%m-%d %H:%M", &e);
			std::cout << " to " << date_and_time;
		}
	}
	std::cout << "\t";
}

void print_zone(const database &x, int index) {
	std::cout << index << "\t";
	if (index <= 0 || index > x.events.size()) { std::cerr << "Invalid zone index" << std::endl; return; }
	std::cout << x.zones[index - 1].tzid() << std::endl;
}

void print_zones(const database &x) {
	for (int i = 0; i < x.zones.size(); i++) 
		print_zone(x, i + 1);
}

void print_summary(const database &x, int index) {
	std::cout << index << "\t";
	if (index <= 0 || index > x.events.size()) { std::cerr << "Invalid event index" << std::endl; return; }
	print_date_and_time(x, x.events[index - 1].dtstart, x.events[index - 1].dtend);
	if (x.events[index - 1].summary == "") std::cout << "No summary given" << std::endl;
	else std::cout << x.events[index - 1].summary << std::endl;
}

void print_summaries(const database &x) {
	for (int i = 0; i < x.events.size(); i++) 
		print_summary(x, i + 1);
}

void print_description(const database &x, int index) {
	std::cout << index << "\t";
	if (index <= 0 || index > x.events.size()) { std::cerr << "Invalid event index" << std::endl; return; }
	else if (x.events[index - 1].description == "") std::cout << "No description given" << std::endl;
	else std::cout << x.events[index - 1].description << std::endl;
}

void print_descriptions(const database &x) {
	for (int i = 0; i < x.events.size(); i++) 
		print_description(x, i + 1);
}

void print_participants(const database &x, int index) {
	std::cout << index << "\t";
	
	//print participants
	if (index <= 0 || index > x.events.size()) { std::cerr << "Invalid event index" << std::endl; return; }
	else {
		if (x.events[index - 1].organizer == "") std::cout << "Organizer:\tNo organizer named" << std::endl; 
		else if (x.events[index - 1].organizer_cn != "") std::cout << "Organizer:\t" << x.events[index - 1].organizer_cn << std::endl;
		else std::cout << "Organizer:\t" << x.events[index - 1].organizer << std::endl;
		if (x.events[index - 1].attendee == "") std::cout << "\tAttendee:\tNo attendee named" << std::endl;
		else std::cout << "\tAttendee:\t" << x.events[index - 1].attendee << std::endl;
	}
}

void print_all_participants(const database &x) {
	for (int i = 0; i < x.events.size(); i++) 
		print_participants(x, i + 1);
}

void print_more_than_one(const database &x, int index) {
	std::cout << index << "\t";
	if (index <= 0 || index > x.events.size()) { std::cerr << "Invalid event index" << std::endl; return; }
	if (cmdline.show_events_list) {
		if (x.events[index - 1].summary == "") std::cout << "Summary:\tNo summary given" << std::endl;
		else print_date_and_time(x, x.events[index - 1].dtstart, x.events[index - 1].dtend), 
		     std::cout << "\n\tSummary:\t" << x.events[index - 1].summary << std::endl;	
	} 
	if (cmdline.show_participants) {
		if (cmdline.show_events_list) std::cout << "\t";
		if (x.events[index - 1].organizer == "") std::cout << "Organizer:\tNot mentioned" << std::endl; 
		else if (x.events[index - 1].organizer_cn != "") std::cout << "Organizer:\t" << x.events[index - 1].organizer_cn << std::endl;
		else std::cout << "Organizer:\t" << x.events[index - 1].organizer << std::endl;
		if (x.events[index - 1].attendee == "") std::cout << "\tAttendee:\tNot mentioned" << std::endl;
		else std::cout << "\tAttendee:\t" << x.events[index - 1].attendee << std::endl; 
	}
	if (cmdline.show_description_list) {
		if (x.events[index - 1].description == "") std::cout << "\tDescription:\tNo description given" << std::endl;
		else std::cout << "\tDescription:\t" << format_string(x.events[index - 1].description, 2) << std::endl;	
	}
	if (index != x.events.size()) std::cout << std::endl;
}

void print_more_than_one_all(const database &x) {
	for (int i = 0; i < x.events.size(); i++) {
		print_more_than_one(x, i + 1);
	}
}

//function which tells if the use chose more than one option (is expandable as we get more boolean variables)
bool more_than_one(bool a, bool b, bool c) {
	if (a && b || a && c || b && c) return true;
	return false;
}

int main(int argc, char **argv) {

	try {
		parse_cmdline(argc, argv);
	} catch (...) {
		std::cerr << "Invalid Argument: -e requires a numeric argument" << std::endl;
		return -1;
	}

	database x(cmdline.input_name);

	int event_index = cmdline.event_index; //the user will decide this number, that's just an interim solution until I know how to get
	//the number from the cmdline-parser

	//if the -e flag is entered, the other flags only apply to the event with the entered index
	if (cmdline.show_event_nr) {

		if (more_than_one(cmdline.show_events_list, cmdline.show_description_list, cmdline.show_participants))
			print_more_than_one(x, event_index);
		else if (cmdline.show_events_list) print_summary(x, event_index);
		else if (cmdline.show_description_list) print_description(x, event_index);
		else if (cmdline.show_participants) print_participants(x, event_index);

	} else {

		if (cmdline.show_zone_list) print_zones(x);
		if (more_than_one(cmdline.show_events_list, cmdline.show_description_list, cmdline.show_participants))
			print_more_than_one_all(x);
		else {
			if (cmdline.show_events_list) print_summaries(x);
			if (cmdline.show_description_list) print_descriptions(x);
			if (cmdline.show_participants) print_all_participants(x);
		}

	}

	return 0;
}


