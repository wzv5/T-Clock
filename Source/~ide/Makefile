.PHONY: all clean clock options

TARGET = 

all: clock options

clean: TARGET = clean
clean: all

clock:
	cd .. && make -f Makefile.unix $(TARGET)

options:
	make -f linux-options.make $(TARGET)
