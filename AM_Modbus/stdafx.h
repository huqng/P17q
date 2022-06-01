// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#include <mutex>

#define WIN32_LEAN_AND_MEAN             //  从 Windows 头文件中排除极少使用的信息
// Windows 头文件:
#include <windows.h>
#include <process.h>
#include <WinBase.h>
#include <mmsystem.h>

// TODO: 在此处引用程序需要的其他头文件
#include "../AM_Client/AM_IClient.h"
#include "../AM_SerialPort/AM_ISerialPort.h"

#include "Global_Define.h"

#pragma comment(lib, "Winmm.lib")

//#ifdef _DEBUG
//#pragma comment(lib, "../../../Bin/Debug/AM_Client.lib")
//#pragma comment(lib, "../../../Bin/Debug/AM_SerialPort.lib")
//#else
//#pragma comment(lib, "../../../Bin/Release/AM_Client.lib")
//#pragma comment(lib, "../../../Bin/Release/AM_SerialPort.lib")
//#endif

using std::mutex;
