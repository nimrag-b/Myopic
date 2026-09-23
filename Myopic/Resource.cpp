#include "Resource.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static uint32_t next_id() {
	static uint32_t glob_ID = 0; //make sure that zero is never assigned to a valid object

	return InterlockedIncrement(&glob_ID);
}

IResource::IResource() {
	_ID = next_id();
}
