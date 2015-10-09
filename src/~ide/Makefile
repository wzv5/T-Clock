.PHONY: all clean release debug autoversion dll clock options calendar

TARGET = 
MAKE = make
TYPE = linux

ifeq ($(OS)$(PWD),Windows_NT)
	MAKE = mingw32-make
	TYPE = gcc
endif

all: autoversion dll clock options calendar

clean: TARGET = clean
clean: all

release: TARGET = release
release: all

debug: TARGET = debug
debug: all

autoversion:
	$(MAKE) -f $(TYPE)-autoversion.make $(TARGET)

dll: autoversion
	$(MAKE) -f $(TYPE)-dll.make $(TARGET)

clock: dll
	$(MAKE) -f $(TYPE)-clock.make $(TARGET)

options: dll
	$(MAKE) -f $(TYPE)-options.make $(TARGET)

calendar: dll
	$(MAKE) -f $(TYPE)-calendar.make $(TARGET)
