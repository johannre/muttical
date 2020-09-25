if HAVE_GTEST
noinst_PROGRAMS = tests

tests_SOURCES = main.cpp \
				../src/database.cpp \
				test_db.h 

# debian libgtest is broken
tests_CXXFLAGS = -pthread

TESTS = tests
endif
