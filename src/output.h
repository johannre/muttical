#ifndef _MUTTICAL_OUTPUT_H_
#define _MUTTICAL_OUTPUT_H_

#include "cmdline.h"
#include "database.h"

#include <iostream>
#include <cmath> 
#include <iomanip>
#include <fstream>
#include <filesystem>
#include <string>
#include <regex>
#include <map>
#include <functional>

void save_event_in_ics(const database &x, int index, std::string filename, bool print_first, bool print_last, bool append, bool force);

void save_events_in_ics(const database &x, std::string f);

bool whole_date_print(const database &x);

std::string format_string(const std::string &str, int num_tabs);

void print_date_and_time(const database &x, const std::tm &d, const std::tm &e);

void print_zone(const database &x, int index);

void print_zones(const database &x);

void print_summary(const database &x, int index);

void print_summaries(const database &x);

void print_description(const database &x, int index);

void print_descriptions(const database &x);

void print_participants(const database &x, int index);

void print_all_participants(const database &x);

void print_more_than_one(const database &x, int index, bool &error_in_functions);

void print_more_than_one_all(const database &x);

bool more_than_one(bool a, bool b, bool c, bool d, bool e);

void interactive_mode(const database &x);

void print_near_event(const database &x, int index, bool other_options);

void print_near_events(database &x);

#endif //_MUTTICAL_OUTPUT_H_
