USE_DEBUG = NO
USE_64BIT = NO
USE_UNICODE = NO
USE_CLANG = NO
# use -static for clang/llvm and cygwin/mingw
USE_STATIC = NO

include der_libs\tool_select.mak 

ifeq ($(USE_DEBUG),YES)
CFLAGS=-Wall -ggdb -c
LFLAGS=-mwindows
else
CFLAGS=-Wall -O3 -c
LFLAGS=-s -mwindows
endif
CFLAGS += -Wno-write-strings

# build common code first
CFLAGS += -Ider_libs

IFLAGS += -Ider_libs

CPPSRC=cdtimer.cpp config.cpp about.cpp zplay_audio.cpp

CPPSRC+=der_libs/common_funcs.cpp \
der_libs/common_win.cpp \
der_libs/winmsgs.cpp \
der_libs/statbar.cpp \
der_libs/trackbar.cpp \
der_libs/hyperlinks.cpp 

RCSRC=cdtimer.rc

OBJS = $(CPPSRC:.cpp=.o) rc.o

BIN=cdtimer
BINX=$(BIN).exe 

LIBS=-lgdi32 -lcomctl32 -lwinmm -lzplay

#**************************************************************
#  generic build rules
#**************************************************************
%.o: %.cpp
	$(TOOLS)/$(GNAME) $(CFLAGS) $< -o $@

all: $(BINX)

clean:
	rm -f $(BINX) $(OBJS)

dist:
	rm -f *.zip
	zip $(BIN).zip $(BINX) readme.md libzplay.dll LICENSE.txt
																			
wc:
	wc -l *.cpp *.rc

clint:
	cmd /C "python ..\ClaudeLint.py --exclude der_libs"
	
cppc:
	cmd /C "cppcheck --project=compile_commands.json --std=c++14 --suppressions-list=./.suppress.cppcheck"

check:
	cmd /C "d:\llvm\bin\clang-tidy.exe $(CPPSRC)"

depend:
	makedepend $(IFLAGS) $(CPPSRC)

#**************************************************************
#  build rules for executables                           
#**************************************************************
cdtimer.exe: $(OBJS)
	$(TOOLS)/$(GNAME) $(OBJS) $(LFLAGS) -o $(BINX) $(LIBS) 
#	g++ $(CFLAGS) $(LFLAGS) $^ -o $@ $(LIBS)

#**************************************************************
#  build rules for libraries and other components
#**************************************************************
rc.o: $(RCSRC)
	$(TOOLS)\$(WRNAME) $< -O COFF -o $@
#	windres -i $< -O COFF -o $@

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
