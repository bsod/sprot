INCLUDES=@BOOST_CPPFLAGS@
man_MANS=sprot.1
bin_PROGRAMS=sprot
sprot_SOURCES=sprot.h sprot.cc main.cc sysdep.h sysdep_unix.cc
sprot_LDFLAGS=@BOOST_LDFLAGS@
sprot_LDADD=-lboost_program_options-mt -lboost_thread-mt

check_PROGRAMS = sprotcheck
sprotcheck_SOURCES = sprot.h sprot.cc sprotcheck.cc
sprotcheck_CXXFLAGS = $(CPPUNIT_CFLAGS)
sprotcheck_LDFLAGS = $(CPPUNIT_LIBS) -ldl

TESTS=$(check_PROGRAMS)

EXTRA_DIST=sprot.1 sysdep_windows.cc
