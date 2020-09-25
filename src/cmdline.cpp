#include "config.h"
#include "cmdline.h"

#include <argp.h>
#include <iostream>
#include <cstdlib>
#include <sstream>
#include <cctype>

using namespace std;

const char *argp_program_version = PACKAGE_VERSION;

static char doc[]       = PACKAGE_NAME ": Print VCalendar (ics) related information";
static char args_doc[]  = "<icsfile>";

static struct argp_option options[] = 
{
	// --[opt]		short/const		arg-descr		flag	option-descr
	{ "verbose", 			'v', 	0,         		0, "Be verbose." },
	{ "zones",                      'z', 	0,			0, "Show timezone of event." },
	{ "list-events", 		'l',	0,			0, "Show list of events." },
	{ "event",                      'e',    "event_index",                      0, "Get access to specific event." },
	{ "description",                'd',    0,                      0, "Shows description of each event." },
	{ "participants",               'p',    0,                      0, "Shows organizer and participants." },
	{ 0 }
};

static error_t parse_options(int key, char *arg, argp_state *state)
{
	// call argp_usage to stop program execution if something is wrong
	string sarg;
	if (arg)
		sarg = arg;

	switch (key)
	{
	case 'v':
		cmdline.verbose = true; 
		break;
	case 'z':
		if (cmdline.show_event_nr) argp_usage(state);
		cmdline.show_zone_list = true;
		break;
	case 'l':
		cmdline.show_events_list = true;
		break;
	case 'e':
		if (cmdline.show_zone_list) argp_usage(state);
		cmdline.show_event_nr = true;
		cmdline.event_index = stoi(sarg);
		break;
	case 'd':
		cmdline.show_description_list = true;
		break;
	case 'p':
		cmdline.show_participants = true;
		break;
	case ARGP_KEY_ARG:
		if (state->arg_num >= 2) // too many arguments
			argp_usage(state);
		cmdline.input_name = sarg;
		break;
	default:
		return ARGP_ERR_UNKNOWN;
	}

	return 0;
}

static struct argp parser = { options, parse_options, args_doc, doc };

int parse_cmdline(int argc, char **argv)
{
	int ret = argp_parse(&parser, argc, argv, /*ARGP_NO_EXIT*/0, 0, 0);
	return ret;
}
	
struct cmdline cmdline;

