#include "MString.h"
#include "string.h"

namespace str {
	//allocates at least len+1 bytes for string
	//this memory is zeroed out so strlen will return 0
	char* AllocString(size_t len) {
		return new char[len + 1] {0};
	}

	void FreeString(const char* str) {
		delete[] str;
	}

	char* DupString(const char* str) {
		if (str == nullptr) return nullptr;
		size_t len = strlen(str);
		char* s = AllocString(len);
		return (char*)memcpy(s, str, len);
	}

}