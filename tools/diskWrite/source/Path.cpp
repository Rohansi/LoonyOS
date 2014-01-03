#include <Path.hpp>

void Path::_parse1() {
	mId = ParseMountId(fullPath);

	/*if (mId == err)
		return;*/

	if (mId == curr) {
		_parse2(2);
	} else if (mId == boot) {
		_parse2(1);
	} else {
		/*if (fullPath.length() < 5 || !(fullPath[3] == ':' && fullPath[4] == '/'))
			return;

		_parse2(5);*/

		mId = err; // can't pick drive
	}
}

void Path::_parse2(size_t start) {
	if (start == fullPath.length()) {
		isValid = true;
		return;
	}

	std::string temp;

	for (size_t i = start; i < fullPath.length(); i++) {
		if (fullPath[i] == '/') {
			if (!temp.length() && i != fullPath.length() - 1)
				return;

			directories.push_back(temp);
			temp.clear();
		} else {
			temp.push_back(fullPath[i]);
		}
	}

	if (temp.length())
		fileName = temp;

	_parse3();
}

void Path::_parse3() {
	for (size_t i = 0; i < directories.size(); i++) {
		if (!directories[i].length() || !IsValidName(directories[i]))
			return;
	}

	if (!IsValidName(fileName))
		return;

	isValid = true;
}

mountId ParseMountId(const std::string& path) {
	if (path.length() > 1 && path[0] == '.') {
		if (path[1] != '/')
			return err;

		return curr;
	} else if (path.length() && path[0] == '/') {
		return boot;
	}

	if (path.length() < 3)
		return err;

	if (path[0] == 'h' && path[1] == 'd') {
		return (path[2] >= '0' && path[2] <= '3') ? hd0 + (path[2] - '0') : err;
	} else if (path[0] == 'o' && path[1] == 'd') {
		return (path[2] >= '0' && path[2] <= '3') ? od0 + (path[2] - '0') : err;
	} else {
		return err;
	}
}

bool IsValidName(const std::string& name) {
	for (size_t i = 0; i < name.length(); i++) {
		switch (name[i]) {
			case '\\':
			case '/':
			case ':':
			case '*':
			case '?':
			case '"':
			case '<':
			case '>':
			case '|':
				return false;
		}
	}

	return true;
}
