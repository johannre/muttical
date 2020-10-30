#ifndef _EVENT_ 
#define _EVENT_ 

#include <iostream> 
#include "timezone.h"
#include "lineinfo.h"
#include <vector>
#include <ctime>
#include <cmath>

class Event { 	
	public:
		Event(const std::vector<lineinfo> &x, int start, int end, int index);
		std::string summary; 	
		std::string description; 	
		std::string timezone;
		std::string timezone_line;
		std::string all_information; 	
		std::tm dtstart;
		std::tm dtend;
		int days_between;
		std::string organizer; 
		std::string organizer_cn;
		std::vector<std::string> attendee;
		std::vector<lineinfo> info;
		int start();
		int end();
		int index; //of all events, starting with 1
	private:
		void extract_values();
		void calculate_time_until();
}; 
#endif //_EVENT

