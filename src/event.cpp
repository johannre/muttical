#include "event.h"
#include "database.h"
using namespace std;

constexpr char delete_marker = '\a'; //a random character which doesn't occur in .ics files.

tm extract_duration(const tm &start, string duration);

Event::Event(const vector<lineinfo> &x, int start, int end, int index) : index(index) {

	bool something_else = false;
	string value_of_lineinfo;
	int i = 0;
	for (int j = start; j < end; j++) {
		if (x[j].key == "BEGIN") something_else = true, value_of_lineinfo = x[j].value;
		else if (x[j].key == "END" && something_else && x[j].value == value_of_lineinfo) something_else = false;
		else if (!something_else) {
			info.emplace_back();
			info[i++] = x[j];
		}
	}

	extract_values();
	calculate_time_until();
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

void Event::extract_values() {
	for (lineinfo &i : info) {
		if (i.key == "SUMMARY") {
			summary = i.value;
			format_string_with_escapesequences(summary);
		}
		else if (i.key == "DESCRIPTION") {
			description = i.value; 
			format_string_with_escapesequences(description);
		}
		else if (i.key == "DTSTART") {
			if (strptime(i.value.c_str(), "%Y%m%dT%H%M", &dtstart) == nullptr) {
				strptime(i.value.c_str(), "%Y%m%d", &dtstart);
				dtstart.tm_hour = 0;
				dtstart.tm_min = 0;
			}
			dtstart.tm_sec = 0;
		}
		else if (i.key == "DTEND") {
			if (strptime(i.value.c_str(), "%Y%m%dT%H%M", &dtend) == nullptr) {
				strptime(i.value.c_str(), "%Y%m%d", &dtend);
				dtend.tm_hour = 0;
				dtend.tm_min = 0;
				dtend.tm_sec = 0;
			}
			dtend.tm_sec = 0;
		}
		else if (i.key == "DURATION") {
			dtend = extract_duration(dtstart, i.value);
		}
		else if (i.key == "ORGANIZER") {
			organizer = i.value;
			auto it = i.params.find("CN");
			if (it != i.params.end())
				organizer_cn = it->second;
			format_string_with_escapesequences(organizer);
		}
		else if (i.key == "ATTENDEE") {
			attendee.push_back(i.value);
			format_string_with_escapesequences(attendee[attendee.size() - 1]);
		}
	}

}

//calculates how long the period of time from now until the event is
void Event::calculate_time_until() {
	//getting the current time
	time_t t = std::time(0);
	tm *now = std::localtime(&t);
	now->tm_year;
	now->tm_mon;

	time_t x = mktime(now);
	time_t y = mktime(&dtstart);

	if (x != (time_t)(-1) && y != (time_t)(-1))
		days_between = difftime(y, x) / (60 * 60 * 24);
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

int Event::start() { return days_between; }
