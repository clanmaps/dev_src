AM_CPPFLAGS= -I${TBLIB_ROOT}/include/tbsys \
             -I${ONEV_ROOT}/include/onev \
	     -I${top_srcdir}/include \
	     -I${top_srcdir}/src \
	     -fPIC

LDADD = ${ONEV_LIB_PATH}/libonev.a \
	${TBLIB_ROOT}/lib/libtbsys.a

AM_LDFLAGS=-lpthread -lc -lm  -lrt -lcrypt -lssl -lcrypto #-pg
if COVERAGE
CXXFLAGS+=-fprofile-arcs -ftest-coverage
AM_LDFLAGS+=-lgcov
endif

noinst_LIBRARIES = libcompactsstable.a

libcompactsstable_a_SOURCES = ob_block_membuf.cpp \
			      ob_compactsstable_mem.cpp \
                              ob_compact_row.cpp




EXTRA_DIST =  ob_block_membuf.h \
              ob_compact_obj.h \
              ob_compact_row.h \
              ob_compactsstable_mem.h

clean-local:
	-rm -f *.gcov *.gcno *.gcda
