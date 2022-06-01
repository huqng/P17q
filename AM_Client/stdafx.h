// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             //  从 Windows 头文件中排除极少使用的信息
// Windows 头文件:
#include <windows.h>
#include <tchar.h>
#include <string>
#include <algorithm>
#include <process.h>
#include <WinSock2.h>
#include <vector>
#include <atlconv.h>
#include <MSTcpIP.h>
#include <ws2tcpip.h>
#include <assert.h>

#pragma comment(lib, "WS2_32.lib")
//mswsock.lib
#pragma comment(lib, "mswsock.lib")

#ifdef _UNICODE
#define tstring std::wstring
#else
#define tstring std::string
#endif



// TODO: 在此处引用程序需要的其他头文件
