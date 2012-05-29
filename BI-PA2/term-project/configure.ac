# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([matrixcalc], [1.0.0], [p.janouch@gmail.com])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_LIBOBJ_DIR([src])
AC_CONFIG_HEADERS([config.h])

AC_SUBST([USERNAME], [janoupre])

AC_CANONICAL_HOST
AM_INIT_AUTOMAKE([1.9 foreign subdir-objects dist-xz -Wall -Werror])
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.18])

# Checks for programs.
AC_LANG([C++])
AC_PROG_CXX
AC_PROG_LN_S
AC_PROG_INSTALL

DX_INIT_DOXYGEN(AC_PACKAGE_NAME, [doc/Doxyfile], [doc])

# Checks for libraries.
AC_SEARCH_LIBS([pow], [m])
AX_LIB_READLINE

# Checks for header files.
AC_HEADER_STDBOOL
AC_CHECK_HEADERS([libintl.h locale.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([floor memset pow setlocale],,
	AC_MSG_ERROR([some of the required functions were not found]))

AC_CONFIG_FILES([Makefile po/Makefile.in Makefile.progtest
	config.h.progtest doc/Doxyfile])
AC_OUTPUT

