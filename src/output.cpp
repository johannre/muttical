#include "output.h"

extern bool error;

void print_near_event(const database &x, int index, bool other_options) {

	if (other_options) std::cout << "\t";
	else std::cout << index << "\t";

	const Event &e = x.events[index - 1];

	if (e.days_between >= 0 && e.days_between != 1)
		std::cout << "in " << index << " days";
	else if (e.days_between == 1) 
		std::cout << "in " << index << " day";
	else if (e.days_between != 1)
		std::cout << abs(e.days_between) << " days ago";
	else if (e.days_between == 1)
		std::cout << abs(e.days_between) << " day ago";

	if (!other_options) std::cout << ":\t" << e.summary;
	std::cout << std::endl;

}

void print_near_events(database &x) {

	using namespace std;
	vector<Event *> e;

	for (auto &i : x.events) e.emplace_back(&i);

	//sorting the events by their start-time
	sort(e.begin(), e.end(), [](Event *e1, Event *e2) { return (e1->start() < e2->start()); });

	//printing the upcoming events
	for (int i = 0; i < e.size(); i++) {
		int days = e[i]->start();
		if (days >= 0 && days != 1) 
			cout << e[i]->index << "\tin " << days << " days:\t" << e[i]->summary << endl;
		else if (days == 1)
			cout << e[i]->index << "\tin " << days << " day:\t" << e[i]->summary << endl;
	}

	//printing the finished events
	for (int i = 0; i < e.size(); i++) {
		int days = e[e.size() - 1 - i]->start();
		if (days < 0 && days != -1) 
			cout << e[i]->index << "\t" << abs(days) << " days ago:\t" << e[e.size() - 1 - i]->summary << endl;
		else if (days == -1)
			cout << e[i]->index << "\t" << abs(days) << " day ago:\t" << e[e.size() - 1 - i]->summary << endl;
	}


}

//store an event with its most important information in an own ics file.
void save_event_in_ics(const database &x, int index, std::string filename, bool print_first, bool print_last, bool append, bool force) {
	using namespace std;

	//if the user entered -a/--append, but there is no existing file, the append doesn't affect anything.
	if (!std::filesystem::exists(filename)) append = false;

	if (index <= 0 || index > x.events.size()) { std::cerr << "Invalid events index" << std::endl; return; }

	ofstream out;
	if (force) out.open(filename);
	else out.open(filename, ios_base::app);

	if (print_first && append) {
		//deleting END:VCALENDAR (which is the last line and mustn't be there if we want
		//to append other events from other files).
		regex end_vcalendar_reg("END:\\s*VCALENDAR");
		smatch evcal_match;

		ifstream in(filename);

		string line;
		string file;

		while (getline(in, line))
			if (!regex_search(line, evcal_match, end_vcalendar_reg))
				file += line + "\n";
		
		//clearing the file 
		out.close();
		out.open(filename);

		//writing the version without END:VCALENDAR in the file
		out << file;
	}

	if (print_first && !append) out << "BEGIN:VCALENDAR\n";
	out << "BEGIN:VEVENT\n";
	if (x.events[index - 1].timezone_line != "") {
		out << "BEGIN:VTIMEZONE\n" 
		    << x.events[index - 1].timezone_line
		    << "\nEND:VTIMEZONE\n";
	}
	for (const auto &i : x.events[index - 1].info) 
		out << i.line << endl;
	out << "END:VEVENT\n";
	if (print_last) out << "END:VCALENDAR\n";

	out.close();
	if (cmdline.verbose) std::cout << "Saved event in \"" << filename << "\"" << std::endl;
}

void save_events_in_ics(const database &x, std::string f) {

	const std::string &filename = cmdline.filename;

	//if the filename already exists, the user can't just overwrite
	//the existing file. If he wants to do so, he has to enter
	//-f/--force, if he wants to append, he has to enter -a/--append
	if (std::filesystem::exists(filename)) {
		if (cmdline.force)
			for (int i = 0; i < x.events.size(); i++) {
				if (cmdline.verbose) std::cout << i + 1 << "\t"; 
				save_event_in_ics(x, i + 1, filename, i == 0, i == (x.events.size() - 1), i != 0, i == 0);
			}

		else if (cmdline.append) 
			for (int i = 0; i < x.events.size(); i++) {
				if (cmdline.verbose) std::cout << i + 1 << "\t";
				save_event_in_ics(x, i + 1, filename, i == 0, i == (x.events.size() - 1), true, false);
			}

		else std::cout << "failed to create " << filename << ": file exists" << std::endl, error = true;

	} else
		for (int i = 0; i < x.events.size(); i++) {
			if (cmdline.verbose) std::cout << i + 1 << "\t"; 
			save_event_in_ics(x, i + 1, filename, i == 0, i == (x.events.size() - 1), false, false);
		}
}

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
		if (x.events[index - 1].attendee.size() == 0) 
			std::cout << "\tAttendee:\tNot mentioned" << std::endl;
		else 
			for (const auto &a : x.events[index - 1].attendee)
				std::cout << "\tAttendee:\t" << a << std::endl; 
	}
}

void print_all_participants(const database &x) {
	for (int i = 0; i < x.events.size(); i++) 
		print_participants(x, i + 1);
}

void print_more_than_one(const database &x, int index, bool &error_in_functions) {
	std::cout << index;
	if (index <= 0 || index > x.events.size()) { std::cerr << "\tInvalid event index" << std::endl; return; }
	if (cmdline.show_events_list) {
		std::cout << "\t";
		if (x.events[index - 1].summary == "") std::cout << "Summary:\tNo summary given" << std::endl;
		else print_date_and_time(x, x.events[index - 1].dtstart, x.events[index - 1].dtend), 
			std::cout << "\n\tSummary:\t" << x.events[index - 1].summary << std::endl;	
	} 
	if (cmdline.upcoming) {
		print_near_event(x, index, true);
	}
	if (cmdline.show_participants) {
		std::cout << "\t";
		if (x.events[index - 1].organizer == "") std::cout << "Organizer:\tNot mentioned" << std::endl; 
		else if (x.events[index - 1].organizer_cn != "") std::cout << "Organizer:\t" << x.events[index - 1].organizer_cn << std::endl;
		else std::cout << "Organizer:\t" << x.events[index - 1].organizer << std::endl;
		if (x.events[index - 1].attendee.size() == 0) 
			std::cout << "\tAttendee:\tNot mentioned" << std::endl;
		else 
			for (const auto &a : x.events[index - 1].attendee)
				std::cout << "\tAttendee:\t" << a << std::endl; 
	}
	if (cmdline.show_description_list) {
		std::cout << "\t";
		if (x.events[index - 1].description == "") std::cout << "Description:\tNo description given" << std::endl;
		else std::cout << "Description:\t" << format_string(x.events[index - 1].description, 2) << std::endl;	
	}
	if (cmdline.save_events) {
		bool one = cmdline.event_index != 0; 
		if (std::filesystem::exists(cmdline.filename)) {
			if (cmdline.force)
				save_event_in_ics(x, index, cmdline.filename, index == 1 || one, index == x.events.size() || one, index != 1, index == 1);
			else if ((cmdline.append || index != 1 && !one) && !error_in_functions)
				save_event_in_ics(x, index, cmdline.filename, index == 1 || one, index == x.events.size() || one, true, false);
			else std::cout << "\tfailed to create " << cmdline.filename << ": file exists" << std::endl, error = error_in_functions = true;

		} else
			save_event_in_ics(x, index, cmdline.filename, index == 1 || one, index == x.events.size() || one, index != 1 || !one, false);
	} 
	if (index != x.events.size() && cmdline.event_index != 0) std::cout << std::endl;
}

void print_more_than_one_all(const database &x) {
	//if something went wrong in one of the functions, it turns true which influences some functions 
	bool error_in_functions = false;
	for (int i = 0; i < x.events.size(); i++) {
		print_more_than_one(x, i + 1, error_in_functions);
	}
}

//function which tells if the use chose more than one option (is expandable as we get more boolean variables)
bool more_than_one(bool a, bool b, bool c, bool d, bool e) {
	std::vector<bool> x { a, b, c, d, e };

	int n = 0; 
	for (bool b : x) if (b) n++;

	return n > 1;
}
