#include "../../pch.h"
#include "string.h"

wchar_t* EDFWString_t::c_str()
{
	if (size < 8) {
		return text;
	}
	else {
		return pText;
	}
}
