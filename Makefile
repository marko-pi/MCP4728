
all: compile

install:
	gcc -o libMCP4728.so -shared -fPIC MCP4728.c
	sudo mv libMCP4728.so /usr/local/lib
	sudo ldconfig

uninstall:
	sudo rm -f /usr/local/lib/libMCP4728.so
	sudo ldconfig

compile:
	gcc -o MCP4728.so -shared -fPIC MCP4728.c

clean:
	rm -f MCP4728.so
