#include "database.h"
#include <chrono>
using namespace std;

constexpr char delete_marker = '\a'; //a random charakter which doesn't occure in .ics files.

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
			zones.emplace_back(i.value);
			zones[zones_index++].index = i.index_in_all_lines;
		}
	}
}

void database::extract_timezone_for_event(vector<pair<int, int>> event_indices) {
	for (auto &z : zones) {
		for (int i = 0; i < event_indices.size(); i++) {
			if (z.index < event_indices[i].second) //if the tzid-information is before the end of the event
				events[i].timezone = z.tzid();
		}
	}
}

//making the written "\n" to a newline character '\n'
void format_string_with_escapesequences(string &str) {
	//to avoid unnecessary check for delete-marker
	bool need_delete_marker_check = false;

	for_each(str.begin(), str.end(), [&need_delete_marker_check](char &c) {
		if ((c == '\\') && (*(&c + 1) == 'n')) c = '\n', *(&c + 1) = '\t';
		else if ((c == '\\') && (*(&c + 1) == 't')) c = '\t', *(&c + 1) = delete_marker, need_delete_marker_check = true;
		else if ((c == '\\') && (*(&c + 1) == 'v')) c = '\v', *(&c + 1) = delete_marker, need_delete_marker_check = true;
		else if ((c == '\\') && (*(&c + 1) == ',')) c = ',', *(&c + 1) = delete_marker, need_delete_marker_check = true;
	});
	
	//if there was a \t which causes the deletion of characters, we search for the markers and remove those
	if (need_delete_marker_check)
		str.erase(remove_if(str.begin(), str.end(), [](const char &c) { if (c == delete_marker) return true; return false; }), str.end());
}

//searches in lines of event
lineinfo *database::find_line(Event &e, const std::string &key) {
	for (auto &i : e.info)
		if (i.key == key) return &i;
	return nullptr;
}

//searcher in all lines 
lineinfo *database::find_line(const std::string &key) {
	for (auto &i : all_info)
		if (i.key == key) return &i;
	return nullptr;
}

//extracts the end time based on a start-time and a duration-string
std::tm extract_duration(const tm &start, string duration) {

	tm t; //this variable will hold the duration extracted from the duration string

	bool reading_w = false;
	bool reading_d = false;
	bool reading_h = false;
	bool reading_min = false;
	bool reading_sec = false;
	reverse(duration.begin(), duration.end());

	int multiplier = 1;
	for (int i = 0; i < duration.size() && duration[i] != 'P'; i++) {
		char c = duration[i];
		
		//checking which time-i-member is read in
		if (c == 'S') reading_sec= true;
		else if (c == 'M') reading_sec = false, reading_min= true, multiplier = 1;
		else if (c == 'H') reading_min = false, reading_h = true, multiplier = 1;
		else if (c == 'D') reading_h = false, reading_d = true, multiplier = 1;
		else if (c == 'W') reading_d = false, reading_w = true, multiplier = 1;
		else if (c == 'T') continue;

		//assigning the int-version of the character to the time-member
		else if (reading_sec) 
			t.tm_sec += multiplier * (c - '0'), multiplier *= 10;
		else if (reading_min) 
			t.tm_min += multiplier * (c - '0'), multiplier *= 10;
		else if (reading_h) 
			t.tm_hour += multiplier * (c - '0'), multiplier *= 10;
		else if (reading_d) 
			t.tm_mday += multiplier * (c - '0'), multiplier *= 10;
		else if (reading_w)
			t.tm_mday += multiplier * (c - '0') * 7, multiplier *= 10;
	}

	//converting the start-time to a time_point (makes adding time possible)
	auto tp = chrono::system_clock::from_time_t(std::mktime(const_cast<tm *>(&start)));;

	//adding the extracted duration to the start-time
	tp += std::chrono::seconds(t.tm_sec);
	tp += std::chrono::minutes(t.tm_min);
	tp += std::chrono::hours(t.tm_hour);
	tp += std::chrono::hours(t.tm_mday * 24); //std::chrono::days exists since C++20

	//converting the time_point back to tm which is the end-time of the event
	time_t convert_back_to_tm = chrono::system_clock::to_time_t(tp);
	tm end = *localtime(&convert_back_to_tm);

	return end;
}

//gets the time the event starts and ends and stores in the time_information string
void database::extract_time_from_event(Event &e) {
	if (lineinfo *i = find_line(e, "DTSTART")) {
		if (strptime(i->value.c_str(), "%Y%m%dT%H%M", &e.dtstart) == nullptr) {
			strptime(i->value.c_str(), "%Y%m%d", &e.dtstart);
			e.dtstart.tm_hour = 0;
			e.dtstart.tm_min = 0;
			e.dtstart.tm_sec = 0;
		}
	}
	if (lineinfo *i = find_line(e, "DTEND")) {
		if (strptime(i->value.c_str(), "%Y%m%dT%H%M", &e.dtend) == nullptr) {
			strptime(i->value.c_str(), "%Y%m%d", &e.dtend);
			e.dtend.tm_hour = 0;
			e.dtend.tm_min = 0;
			e.dtend.tm_sec = 0;
		}
	}
	else if (lineinfo *i = find_line(e, "DURATION")) {
		e.dtend = extract_duration(e.dtstart, i->value);
	}
}

void database::extract_summary(Event &e) {
	if (lineinfo *i = find_line(e, "SUMMARY")) {
		e.summary = i->value;
		format_string_with_escapesequences(e.summary);
	}
}

void database::extract_description(Event &e) {
	if (lineinfo *i = find_line(e, "DESCRIPTION")) {
			e.description = i->value; 
			format_string_with_escapesequences(e.description);
	}
}

void database::extract_organizer(Event &e) {
	if (lineinfo *i = find_line("ORGANIZER")) {
		e.organizer = i->value;
		auto it = i->params.find("CN");
		if (it != i->params.end())
			e.organizer_cn = it->second;
		format_string_with_escapesequences(e.organizer);
	}
}

void database::extract_attendee(Event &e) {
	if (lineinfo *i = find_line(e, "ATTENDEE")) {
		e.attendee = i->value;
		format_string_with_escapesequences(e.attendee);
	}
}

void database::extract_all_events() {
	
	//a vector which holds the first and last index of the lines which belong to one event
	vector<pair<int, int>> event_indices;
	int event_indices_index = 0;

	//getting the begin- and end-index of the event-lines
	for (int i = 0; i < all_info.size(); i++) {
		if (all_info[i].key == "BEGIN" && all_info[i].value == "VEVENT") 
			event_indices.emplace_back(), event_indices[event_indices_index].first = i + 1;
		else if (all_info[i].key == "END" && all_info[i].value == "VEVENT")
			event_indices[event_indices_index++].second = i;
	}

	events.resize(event_indices.size());

	//giving the info-vector of events the fitting size
	for (int i = 0; i < events.size(); i++) 
		events[i].info.resize(event_indices[i].second - event_indices[i].first);

	//storing all infos about the event in the info-vector
	for (int i = 0; i < events.size(); i++) {
		int k = 0;
		vector<lineinfo> &x = events[i].info;
		for (int j = event_indices[i].first; j < event_indices[i].second; j++, k++) {
			x[k].key = all_info[j].key;
			x[k].value = all_info[j].value;
			x[k].line = all_info[j].line;
		}
	}
	smatch begin_match;

	//removing all lines belonging to an other object of the .ics file (starting with begin and ending with end)
	bool something_else = false;
	for (auto &i : events) {
		i.info.erase(remove_if(i.info.begin(), i.info.end(), [&something_else](const lineinfo &li) {
			if (li.key == "BEGIN") { 
				return something_else = true;
			}
			if (li.key == "END" && something_else) {
				something_else = false;
				return true;
			}
			return something_else;
			}), i.info.end());
	}

	for (auto &i : events) {
		extract_description(i);
		extract_summary(i);
		extract_time_from_event(i);
		extract_timezone_for_event(event_indices);
		extract_organizer(i);
		extract_attendee(i);
	}
}
