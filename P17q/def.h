#pragma once
#define QSTR_TO_U8(qstr) (qstr.toStdString())
#define U8_TO_QSTR(u8) (QString::fromUtf8(u8))

#define TYPE_CREAGENT U8_TO_QSTR(u8"通用试剂")
#define TYPE_VREAGENT U8_TO_QSTR(u8"可变试剂")
#define TYPE_WASH U8_TO_QSTR(u8"清洗")
#define ALL_TYPE_LIST {TYPE_CREAGENT ,TYPE_VREAGENT, TYPE_WASH}

#include <string>
using std::string;
string StrToHex(string strIn);