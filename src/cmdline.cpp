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
	{ "event",                      'e',    "event_index",          0, "Get access to specific event." },
	{ "description",                'd',    0,                      0, "Shows description of each event." },
	{ "participants",               'p',    0,                      0, "Shows organizer and participants." },
	{ "save-events",                's',    "<filename>",           0, "Save events in a file." },
	{ "force",                      'f',    0,                      0, "Force to overwrite an existing file with the -s statement." },
	{ "append",                     'a',    0,                      0, "Append the event info to an existing file." }, 
	{ "upcoming",                   'u',    0,                      0, "Shows upcoming and recently finished events." },
	{ "interactive",                'i',    0,                      0, "Activates the interactive mode." },
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
	case 's':
		cmdline.save_events = true;
		cmdline.filename = sarg;
		break;
	case 'f':
		cmdline.force = true;
		break;
	case 'a':
		cmdline.append = true;
		break;
	case 'u':
		cmdline.upcoming = true;
		break;
	case 'i':
		cmdline.interactive = true;
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

