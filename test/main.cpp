#include "test_db.h"

#include <gtest/gtest.h>

using namespace std;



/*! This test is just a suggestion.
 *  I think it might be worthwhile to interprete our database when lifting it a
 *  little out of the muttical-context, i.e. to think about how we would expect
 *  such a class to behave if we had found a library providing it.
 */
TEST(interface, no_file) {
	EXPECT_THROW(database db(""), std::exception); // appropriate exception
}



// 
// testing internals
// (for now I do this via comments, we could also just have separate source
// files that we link together. We should think about that when the tests grow
// in number)
//



const char *extraction_test_input = R"(
BEGIN:VEVENT
ORGANIZER;CN=Stan Smith:MAILTO:ssmith@example.org
END:VEVENT)";



TEST(extraction, attendee) {
	database_test db("");
	string doc = extraction_test_input;
	db.build_from(doc);
	auto events = db.events();
	ASSERT_EQ(events.size(), 1);
	EXPECT_EQ(events[0].organizer, "MAILTO:ssmith@example.org");
	EXPECT_EQ(events[0].organizer_cn, "Stan Smith");
}



int main(int argc, char **argv) {
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
