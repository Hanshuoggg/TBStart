// pch.h: 미리 컴파일된 헤더 파일입니다.
// 아래 나열된 파일은 한 번만 컴파일되었으며, 향후 빌드에 대한 빌드 성능을 향상합니다.
// 코드 컴파일 및 여러 코드 검색 기능을 포함하여 IntelliSense 성능에도 영향을 미칩니다.
// 그러나 여기에 나열된 파일은 빌드 간 업데이트되는 경우 모두 다시 컴파일됩니다.
// 여기에 자주 업데이트할 파일을 추가하지 마세요. 그러면 성능이 저하됩니다.

#ifndef PCH_H
#define PCH_H

// Windows 헤더 파일:
#include <windows.h>

// C 런타임 헤더 파일입니다.
#include <tchar.h>

#include <wininet.h>
#pragma comment(lib, "wininet")

#include <fstream>
#include <deque>

#include <process.h>

#include <algorithm>

#include <msclr/marshal_cppstd.h>

#define DEF_HTTP // http

enum E_Define
{
	WM_USER_GetListDone = WM_USER + 131,
	WM_USER_GetFTPDone = WM_USER + 132,
	WM_USER_ErrorQuit = WM_USER + 133,
};

public enum PatchStep
{
	eSTEP_GET_SERVER_VERSION,
	eSTEP_GET_PATCH_VERSION_LIST,
	eSTEP_GET_FILE_LIST,
	eSTEP_GET_PATCH_FILES,
	eSTEP_PATCH_DONE,
};

#endif //PCH_H
