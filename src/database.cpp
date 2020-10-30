#include "database.h"
#include <chrono>
using namespace std;

constexpr char delete_marker = '\a'; //a random charakter which doesn't occure in .ics files.

void remove_unwanted_characters(string &str) {
	string unwanted_chars = "\r"; //expandable
	if (str.find('\r') != string::npos) {
		//removes all unwanted characters from the given string
		for (int i = 0; i < unwanted_chars.size(); i++)
			str.erase(remove(str.begin(), str.end(), unwanted_chars[i]));
	}
}

database::database(string filename) : filename(filename), 
	                              key_reg_1("^[^;^:]+:\\s*"), //form: KEY:VALUE
	                              key_reg_2("^[^:^;]+;\\s*"), //form: KEY;blabla:VALUE
                                      key_reg_both("^[^:^;]+(;|:)"), //form KEY: or KEY;
	                              keys_we_need({ "DTSTART", "DTEND",
	                                             "SUMMARY", "BEGIN", 
	                                             "END", "DESCRIPTION", 
	                                             "TZID", "ORGANIZER",
						     "ATTENDEE", "DURATION" }) {
	//making a better formatted string out of the document
	ifstream in(filename);
	document = string((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());
	remove_unwanted_characters(document);
	build_from(document);
}

//! This is internal and destructively modifies \c document
void database::build_from(string &document) {
	convert_ics_to_vector(document);
	extract_timezones();
	extract_all_events();
}

//stores the .ics file in the vector all_info and all_line (the unstructured string-version of all_info)
void database::convert_ics_to_vector(std::string &document) { 
	//deletes the continuation-line-newlines and the following spaces (which get marked
	//with the character delete_marker). 
	document.erase(remove_if(document.begin(), document.end(), 
				[&](char &c) { 
					if ((c == '\n') && ((*(&c + 1) == ' ') || (*(&c + 1) == '\t'))) {
						*(&c + 1) = delete_marker;
						return true;
					}
					else if (c == delete_marker) return true;
					return false;
				}), document.end());

	//making a vector of strings which holds each line as one element
	istringstream iss(document);
	string line;
	string key;
	smatch k_match;
	while (getline(iss, line)) {
		regex_search(line, k_match, key_reg_both);
		//only saving the lines we need by comparing with the keys_we_need vector
		if (find(keys_we_need.begin(), keys_we_need.end(), line.substr(k_match.position(), k_match.length() - 1)) != keys_we_need.end()) 
			all_lines.push_back(line);
	}

	all_info.resize(all_lines.size());

	//formatting everything into instances of lineinfo
	for (int i = 0; i < all_lines.size(); i++) {
		auto &line = all_lines[i];
		auto &info = all_info[i];
		info.line = all_lines[i];

		bool in_quote = false;
		bool reading_key = true;
		bool reading_param_name = false;
		bool reading_param_value = false;
		string param_name;
		string param_value;
		bool reading_value = false;

		//saving key and line which holds all the rest except the key
		for (int j = 0; j < line.size(); j++) {
			char c = line[j];
			if (c == '"' && line[j - 1] != '\\' && !in_quote) in_quote = true;
			else if (c == '"' && line[j - 1] != '\\' && in_quote) in_quote = false;

			//if the form is KEY;param_name=param_val:VALUE from here ...
			else if (c == ';' && !in_quote && reading_key)
				reading_key = false, reading_param_name = true;

			else if (c == '=' && !in_quote && reading_param_name)
				reading_param_name = false, reading_param_value = true;

			else if (c == ';' && !in_quote && reading_param_value)
				reading_param_value = false, reading_param_name = true;
			//... until here is extra

			else if (c == ':' && !in_quote && !reading_value/*in case the value contains ':'*/)
				reading_key = false, reading_param_value = false,reading_value = true;

			else if (reading_key) info.key += c;
			else if (reading_param_name) param_name += c;
			else if (reading_param_value) param_value += c; 
			else if (reading_value) info.value += c;
		}
		info.index_in_all_lines = i;
		info.params[param_name] = param_value;
	}
}

void database::extract_timezones() {
	int zones_index = 0;
	for (auto &i : all_info) {
		if (i.key == "TZID") {
			zones.emplace_back(i.value, i.line); 
			zones[zones_index++].index = i.index_in_all_lines;
		}
	}
}

//searcher in all lines 
lineinfo *database::find_line(const std::string &key) {
	for (auto &i : all_info)
		if (i.key == key) return &i;
	return nullptr;
}

void database::extract_timezone_for_event(Event &e, int end) {
	for (auto &z : zones)
		if (z.index < end) {
			e.timezone = z.tzid();
			e.timezone_line = z.timezone();
		}
}

void database::extract_all_events() {
	
	//a vector which holds the first and last index of the lines which belong to one event
	int event_indices_first;

	//getting the begin- and end-index of the event-lines
	for (int i = 0; i < all_info.size(); i++) {
		if (all_info[i].key == "BEGIN" && all_info[i].value == "VEVENT") 
			event_indices_first = i;
		else if (all_info[i].key == "END" && all_info[i].value == "VEVENT") {
			events.emplace_back(all_info, ++event_indices_first, i, events.size() + 1);
			extract_timezone_for_event(events[events.size() - 1], i);
		}
	}

}
