#pragma once

#include "src/database.h"

class database_test : public database {
public:
	using database::database;
	using database::build_from;

	std::vector<Event>& events() { return database::events; }
};


