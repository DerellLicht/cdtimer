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
#LiFLAGS += -DWINVER=0x0501

IFLAGS += -Ider_libs

LINTFILES=lintdefs.cpp lintdefs.ref.h

CPPSRC=cdtimer.cpp config.cpp about.cpp zplay_audio.cpp

CPPSRC+=der_libs/common_funcs.cpp \
der_libs/common_win.cpp \
der_libs/winmsgs.cpp \
der_libs/statbar.cpp \
der_libs/trackbar.cpp \
der_libs/hyperlinks.cpp 

RCSRC=cdtimer.rc

OBJS = $(CPPSRC:.cpp=.o) rc.o
BINS=cdtimer.exe 

LIBS=-lgdi32 -lcomctl32 -lwinmm -lzplay

#**************************************************************
#  generic build rules
#**************************************************************
%.o: %.cpp
	g++ $(CFLAGS) -c $< -o $@

all: $(BINS)

clean:
	rm -f $(BINS) $(OBJS)

dist:
	rm -f *.zip
	zip cdtimer.zip cdtimer.exe readme.md libzplay.dll
																			
wc:
	wc -l *.cpp *.rc

check:
	cmd /C "d:\llvm\bin\clang-tidy.exe $(CPPSRC)"

lint:
	c:\lint9\lint-nt +v -width(160,4) $(LiFLAGS) +fcp -ic:\lint9 mingw.lnt -os(_lint.tmp) $(LINTFILES)  cdtimer.rc $(CPPSRC)

depend:
	makedepend $(IFLAGS) $(CPPSRC)

#**************************************************************
#  build rules for executables                           
#**************************************************************
cdtimer.exe: $(OBJS)
	g++ $(CFLAGS) $(LFLAGS) $^ -o $@ $(LIBS)

#**************************************************************
#  build rules for libraries and other components
#**************************************************************
rc.o: $(RCSRC)
	windres -i $< -O COFF -o $@

# DO NOT DELETE

cdtimer.o: resource.h der_libs/common.h der_libs/commonw.h cdtimer.h
cdtimer.o: der_libs/statbar.h der_libs/winmsgs.h der_libs/trackbar.h
cdtimer.o: version.h
config.o: der_libs/common.h cdtimer.h
about.o: resource.h version.h der_libs/hyperlinks.h
der_libs/common_funcs.o: der_libs/common.h
der_libs/common_win.o: der_libs/common.h der_libs/commonw.h
der_libs/statbar.o: der_libs/common.h der_libs/commonw.h der_libs/statbar.h
der_libs/trackbar.o: der_libs/trackbar.h
der_libs/hyperlinks.o: der_libs/iface_32_64.h der_libs/hyperlinks.h
