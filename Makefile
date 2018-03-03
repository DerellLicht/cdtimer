#SHELL=cmd.exe
USE_DEBUG = NO

ifeq ($(USE_DEBUG),YES)
CFLAGS=-Wall -ggdb
LFLAGS=-mwindows
else
CFLAGS=-Wall -O3
LFLAGS=-s -mwindows
endif
CFLAGS += -Wno-write-strings

# build common code first
CFLAGS += -I./der_libs
LiFLAGS += -I./der_libs
LiFLAGS += -I.
LiFLAGS += -DWINVER=0x0501

CPPSRC=der_libs/common_funcs.cpp \
der_libs/common_win.cpp \
der_libs/winmsgs.cpp \
der_libs/statbar.cpp \
der_libs/trackbar.cpp

CPPSRC+=cdtimer.cpp config.cpp 

RCSRC=anacomm.rc

OBJS = $(CPPSRC:.cpp=.o) rc.o
BINS=cdtimer.exe 

%.o: %.cpp
	g++ $(CFLAGS) -c $< -o $@

#**************************************************************
#  generic build rules
#**************************************************************
all: $(BINS)

clean:
	rm -f $(BINS) *.o

dist:
	rm -f *.zip
	zip cdtimer.zip cdtimer.exe cdtimer.txt
																			
lint:
	c:\lint9\lint-nt +v -width(160,4) $(LiFLAGS) +fcp -ic:\lint9 mingw.lnt -os(_lint.tmp) lintdefs.cpp cdtimer.rc $(CPPSRC)

depend:
	makedepend $(CPPSRC)

#**************************************************************
#  build rules for executables                           
#**************************************************************
cdtimer.exe: $(OBJS)
	g++ $(CFLAGS) $(LFLAGS) $^ -o $@ -lgdi32 -lcomctl32 -lwinmm

#**************************************************************
#  build rules for libraries and other components
#**************************************************************
rc.o: cdtimer.rc
	windres -i $< -O COFF -o $@

# DO NOT DELETE

der_libs/common_funcs.o: der_libs/common.h
der_libs/common_win.o: der_libs/common.h der_libs/commonw.h
der_libs/statbar.o: der_libs/common.h der_libs/commonw.h der_libs/statbar.h
der_libs/trackbar.o: der_libs/trackbar.h
cdtimer.o: resource.h der_libs/common.h der_libs/commonw.h cdtimer.h
cdtimer.o: der_libs/statbar.h der_libs/trackbar.h
config.o: der_libs/common.h cdtimer.h
