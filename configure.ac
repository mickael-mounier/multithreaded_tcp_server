AC_INIT([multithreaded_tcp_server], [1.0], [mickael.mounier@free.fr])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AC_CONFIG_MACRO_DIR([m4])
cxxflags_save="$CXXFLAGS"
AC_PROG_CXX
CXXFLAGS="$cxxflags_save"
AC_LANG([C++])
AX_CXX_COMPILE_STDCXX_11([],[mandatory])
AC_CHECK_LIB([pthread], [pthread_create], [], [], [])
AC_CONFIG_HEADERS([config.hh])
AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
