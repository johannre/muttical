#include "output.h"

using namespace std;

using command = map<string, function<void(const database &x)>>;
using command_single_event = map<string, function<void(const database &x, int index)>>;

vector<string> quit_commands { "q", "q", "quit", "exit" };

int contains_string(vector<string> input_words ,const string &str) {
	for (int i = 0; i < input_words.size(); i++) if (input_words[i] == str) return i;
	return -1;
}

void interactive_mode(const database &x) {

	//the possible commands for all events
	command c;
	c["l"] = print_summaries;
	c["list-events"] = print_summaries;
	c["d"] = print_descriptions;
	c["description"] = print_descriptions;
	c["p"] = print_all_participants;
	c["participants"] = print_all_participants;
	c["z"] = print_zones;
	c["zones"] = print_zones;

	//the possible commands for single events
	command_single_event cse;
	cse["l"] = print_summary;
	cse["list-events"] = print_summary;
	cse["d"] = print_description;
	cse["description"] = print_description;
	cse["p"] = print_participants;
	cse["participants"] = print_participants;

	string input;
	vector<string> input_words; //a vector which holds each entered word separated

	bool all_events = true;
	int event_index = -1;

	cout << "muttical > ";

	while(getline(cin, input)) { //stops if the user enters Ctrl and D

		//parsing the input
		stringstream stream(input);
		int num_of_words = distance(istream_iterator<string>(stream), istream_iterator<string>());
		input_words.resize(num_of_words);
		int i = 0;
		stringstream sstream(input);
		while (sstream.good() && i < num_of_words) sstream >> input_words[i++];

		//checking if the user wants to quit the interactive mode ...
		if (find(quit_commands.begin(), quit_commands.end(), input) != quit_commands.end()) break;
		//... if not: 

		auto option = c.find(input);
		auto option_single_event = cse.find(input);

		//executing the matching functions 
		if (input_words.size() == 2 && input_words[0] == "select" && input_words[1] == "all") all_events = true;
		else if (input_words[0] == "select") {
			if (input_words.size() == 2) {
				all_events = false; 
				try { event_index = stoi(input_words[1]); } //might throw an error if the third string isn't a number 
				catch (...) { 
					cout << "muttical: invalid input '" << input << "'." << endl 
						<< "Enter 'select all' or 'select <event-nr>'" << endl; 
				}
			}
			else cout << "muttical: invalid command '" << input << "'." << endl << "Enter 'select all' or 'select <event-nr>'" << endl;
		}
		else if((option != end(c) && all_events) || /*because there's no single-event-option for this:*/input == "z" || input == "zones") 
			(option->second)(x);
		else if(option_single_event != end(cse) && !all_events) 
			(option_single_event->second)(x, event_index);
		else if (input_words[0] == "save-events" || input_words[0] == "s") {
			if (input_words.size() < 2) {
				cout << "muttical: option requires an argument 's'" << endl
					<< "Try 'help' or 'usage' for more information." << endl;
			}
			else {
				const string &filename = input_words[1];

				if (contains_string(input_words, "a") != -1) cmdline.append = true; 
				else if (contains_string(input_words, "append") != -1) cmdline.append = true;
				else if (contains_string(input_words, "f") != -1) cmdline.force = true;
				else if (contains_string(input_words, "force") != -1) cmdline.force = true;

				if (all_events) save_events_in_ics(x, filename);
				else {
					if (filesystem::exists(filename)) {
						if (cmdline.force) 
							save_event_in_ics(x, event_index, filename, true, true, false, true);
						else if (cmdline.append)
							save_event_in_ics(x, event_index, filename, true, true, true, false);
						else cout << "failed to create " << filename << ": file exists" << endl;
					}
					else {
						save_event_in_ics(x, event_index, filename, true, true, cmdline.append, cmdline.force);
					}
				}
			}
		} 
		else if (input == "usage") {
			cout << "Usage: muttical [e event_index] [s <filename>] [append]" << endl
				<< "\t[description] [force] [list-events] [participants]" << endl 
				<< "\t[save-events <filename>] [zones] [setup all]" << endl 
				<< "[setup event <event-index>] [help] [usage]" << endl;
		}
		else if (input == "help" || input == "?") {
			cout << "Usage: muttical [OPTION...] <icsfile>" << endl 
				<< "muttical: Print VCalendar (ics) related information" << endl 
				<< endl 
				<< "a, append                    Append the event info to an existing file." << endl 
				<< "d, description               Shows description of each event." << endl
				<< "f, force                     Force to overwrite an existing file with the s" << endl
				<< "                             statement." << endl
				<< "l, list-events               Show list of events." << endl
				<< "p, participants              Shows organizer and participants." << endl
				<< "s, save-events=<filename>    Save events in a file." << endl
				<< "   select                    Have access to one special event or all events." << endl
				<< "z, zones                     Show timezone of event." << endl
				<< "?, help                      Give this help list" << endl
				<< "   usage                     Give a short usage message" << endl
				<< endl
				<< "Mandatory or optional arguments to long options are also mandatory or optional" << endl
				<< "for any corresponding short options." << endl;
		}
		else if (input == "clear") cout << "\033[2J\033[1;1H"; //ANSI-escape-code which clears the console. Also possible: system("clear"), but doesn't look that cool ;)
		else cout << "muttical: invalid option '" << input << "'" << endl << "Try 'help' or 'usage' for more information." << endl;

		cout << "muttical > ";
	}

}
