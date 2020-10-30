AC_INIT([muttical], [0.1], [kai selgrad])

AC_CONFIG_SRCDIR([src/muttical.cpp])
AC_CONFIG_HEADERS(config.h)
AC_CONFIG_AUX_DIR([auxx])

AM_INIT_AUTOMAKE(subdir-objects)

AC_PROG_CC
AC_PROG_CXX
AC_LANG([C++])
AX_CXX_COMPILE_STDCXX(17)

AC_CHECK_LIB([gtest], [main], [], [AC_MSG_WARN([Testing will not be supported]) ; muttical_has_gtest=false])
AM_CONDITIONAL(HAVE_GTEST, [test x$muttical_has_gtest != xfalse])

AC_CONFIG_FILES([Makefile src/Makefile test/Makefile])

AC_OUTPUT
