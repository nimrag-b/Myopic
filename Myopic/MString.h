#pragma once

namespace str {
	//allocates at least len+1 bytes for string
	//this memory is zeroed out so strlen will return 0
	char* AllocString(size_t len);

	void FreeString(const char* str);
	char* DupString(const char* str);
}


