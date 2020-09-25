#ifndef _EVENT_ 
#define _EVENT_ 

#include <iostream> 
#include "timezone.h"
#include "lineinfo.h"
#include <vector>
#include <ctime>

struct Event { 	
	std::string summary; 	
	std::string description; 	
	std::string timezone;
	std::string all_information; 	
	std::tm dtstart;
	std::tm dtend;
	std::string organizer;
	std::string organizer_cn;
	std::string attendee;
	std::vector<lineinfo> info;
}; 
#endif //_EVENT

