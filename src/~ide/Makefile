.PHONY: all clean clock options

TARGET = 

all: clock options

clean: TARGET = clean
clean: all

clock:
	make -f linux-clock.make $(TARGET)

options:
	make -f linux-options.make $(TARGET)
