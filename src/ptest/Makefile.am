include $(top_srcdir)/src/Makefile.include

AM_CPPFLAGS += -I$(top_srcdir)/src

bin_PROGRAMS = dacrs_ptest

TESTS = test

# test_bitcoin binary #
dacrs_ptest_CPPFLAGS = $(AM_CPPFLAGS) $(TESTDEFS)
dacrs_ptest_LDADD = $(LIBBITCOIN_SERVER) \
               $(LIBBITCOIN_WALLET)   \
			   $(LIBBITCOIN_CLI) \
			   $(LIBBITCOIN_COMMON) \
			   $(LIBLEVELDB) \
			   $(LIBMEMENV) \
               $(BOOST_LIBS) \
			   $(BOOST_UNIT_TEST_FRAMEWORK_LIB)
#if ENABLE_WALLET
#dacrs_ptest_LDADD += $(LIBBITCOIN_WALLET)
#endif
dacrs_ptest_LDADD += $(BDB_LIBS)

dacrs_ptest_SOURCES = \
  p_test.cpp \
  main_tests.cpp \
  miner_tests.cpp \
  updateaccount_tests.cpp \
  mining_tests.cpp	\
  regscript_test.cpp \
  darkanony_ptest.cpp \
  create_tx_tests.cpp \
  CDarkAndAnony.cpp	\
  ../test/SysTestBase.cpp \
  CycleSesureTrade_tests.cpp	\
  CycleTestBase.cpp	\
  CycleTestManger.cpp \
  sesuretrade_test.cpp	\
  CycleTestManger.h	\
  CycleTestBase.h	\
  CycleSesureTrade_tests.h	\
  CDarkAndAnony.h	\
  ../test/SysTestBase.h \
  CycleP2PBet_test.cpp \
  CycleP2PBet_test.h
  