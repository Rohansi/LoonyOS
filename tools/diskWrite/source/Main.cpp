#include <iostream>
#include <fstream>
#include <Script.hpp>

int main(int argc, char* argv[]) {
	if (argc < 2) {
		std::cout << "Usage: dskimg <Script>" << std::endl;
		return 0;
	}

    try {
        Script script;
        script.Run(argv[1]);
    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }

	return 1;
}
