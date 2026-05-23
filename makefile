TARGET = space_escape
OBJS = main.o

CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS)
ASFLAGS = $(CFLAGS)

LIBS = -lpspdebug -lpspdisplay -lpspctrl

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = Space Escape
PSP_EBOOT_ICON = ICON0.PNG

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
