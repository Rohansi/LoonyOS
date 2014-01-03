#include <iostream>
#include <fstream>
#include <Script.hpp>

int line;

void lineHook(lua_State* L, lua_Debug* debug) {
    line = debug->currentline;
}

int main(int argc, char* argv[]) {
	if (argc < 2) {
		std::cout << "Usage: dskimg <Script>" << std::endl;
		return 0;
	}

    try {
        Script script;
        script.Run(argv[1], lineHook);
    } catch (std::exception& e) {
        std::cout << "Line " << line << ": " << e.what() << std::endl;
    }

	return 1;
}
