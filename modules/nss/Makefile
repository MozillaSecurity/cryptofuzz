all: module.a

CXXFLAGS += -Wall -Wextra -Werror -std=c++17 -I ../../include -I ../../fuzzing-headers/include -DFUZZING_HEADERS_NO_IMPL

NSS_LIB_PATH = $(abspath $(NSS_NSPR_PATH))/dist/Debug/lib

# NSS spreads its static build over a set of archives that changes with the NSS
# version, the target architecture and the build flags. freebl alone ships its
# hardware-accelerated and post-quantum code as separate libraries
# (libhw-acc-crypto-avx*, libghash-aes-x86_c_lib, libgcm, libintel-gcm-wrap*,
# libsha-x86_c_lib, libpqcwrap_static, libcrux, ...), and naming them one by one
# has broken this module every time NSS added, split or renamed one. Take
# whatever NSS installed instead, and skip only what must not be mixed in:
#
#   lib<x>.a next to lib<x>_static.a - the copy built for the shared library,
#       which defines the same symbols as its _static counterpart (libfreebl,
#       libpk11wrap, libsoftokn, libpqcwrap, ...)
#   *-nodepend*                      - freebl's FREEBL_NO_DEPEND variants,
#       likewise duplicates of the libraries we do want
#   *-testlib.a                      - test-only libraries
#   libsqlite.a                      - NSS's vendored SQLite; the system one is
#       linked below
NSS_ALL_LIBS = $(notdir $(wildcard $(NSS_LIB_PATH)/*.a))
NSS_SKIP_LIBS = \
	$(patsubst %_static.a,%.a,$(filter %_static.a,$(NSS_ALL_LIBS))) \
	$(foreach lib,$(NSS_ALL_LIBS),$(if $(findstring -nodepend,$(lib)),$(lib))) \
	$(filter %-testlib.a,$(NSS_ALL_LIBS)) \
	libsqlite.a
NSS_LIBS = $(addprefix $(NSS_LIB_PATH)/,$(filter-out $(NSS_SKIP_LIBS),$(NSS_ALL_LIBS)))

module.a: module.o bn_ops.o
	test $(NSS_NSPR_PATH)
	test -n "$(strip $(NSS_LIBS))"

	rm -rf module.a
	rm -rf tmp/

	set -e; for lib in $(NSS_LIBS); do \
		mkdir -p tmp/$$(basename $$lib .a); \
		( cd tmp/$$(basename $$lib .a) && ar x $$lib ); \
	done

	ar qc module.a module.o bn_ops.o
	find tmp/ -name '*.o' -print0 | xargs -0 ar q module.a

	rm -rf tmp/
	ranlib module.a

poc : poc.cpp module.a
	test $(NSS_NSPR_PATH)
	$(CXX) $(CXXFLAGS) \
		-I $(NSS_NSPR_PATH)/dist/public/nss \
		-I $(NSS_NSPR_PATH)/dist/Debug/include/nspr \
		-I $(NSS_NSPR_PATH)/nss/lib/freebl \
		-I $(NSS_NSPR_PATH)/nss/lib/freebl/mpi \
		-I $(NSS_NSPR_PATH)/nss/lib/pk11wrap \
		-I $(NSS_NSPR_PATH)/nss/cpputil \
		poc.cpp \
		module.a \
		-lsqlite3 \
		-o poc

bn_ops.o: bn_ops.cpp bn_ops.h
	test $(NSS_NSPR_PATH)
	$(CXX) $(CXXFLAGS) -I $(NSS_NSPR_PATH)/dist/public/nss -I $(NSS_NSPR_PATH)/dist/Debug/include/nspr -I $(NSS_NSPR_PATH)/nss/lib/freebl -I $(NSS_NSPR_PATH)/nss/lib/freebl/mpi -c bn_ops.cpp -o bn_ops.o

module.o: module.cpp module.h bn_ops.h
	test $(NSS_NSPR_PATH)
	$(CXX) $(CXXFLAGS) -I $(NSS_NSPR_PATH)/dist/public/nss -I $(NSS_NSPR_PATH)/dist/Debug/include/nspr -I $(NSS_NSPR_PATH)/nss/lib/freebl -I $(NSS_NSPR_PATH)/nss/lib/pk11wrap -I $(NSS_NSPR_PATH)/nss/cpputil -fPIC -c module.cpp -o module.o

clean:
	rm -rf *.o module.a
