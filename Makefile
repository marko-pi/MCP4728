

all: MCP4728

install: MCP4728
	cp -a MCP4728.so /usr/local/lib
	cp -a MCP4728.h /usr/local/include

uninstall:
	rm -f /usr/local/lib/MCP4728.so
	rm -f /usr/local/include/MCP4728.h

MCP4728:
	gcc -o MCP4728.so -shared -fPIC MCP4728.c

clean:
	rm -f MCP4728.so
