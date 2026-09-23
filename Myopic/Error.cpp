#include "GameError.h"
#include <WinUser.h>

namespace err {
	void ErrorWithMessage(const wchar_t* msg, const wchar_t* caption) {
		MessageBox(NULL, msg, caption, MB_OK | MB_ICONERROR);
	}
}
