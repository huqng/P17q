#pragma once
#include "stdafx.h"

typedef map<int, double> MultiAxisCoord;

#define STR_SHELF_POS(i) (QString::fromUtf8(u8"玻片架%1提架").arg(i).toStdString())
#define STR_SHELF_HEIGHT(i) (QString::fromUtf8(u8"玻片架%1提放架高度").arg(i).toStdString())
#define STR_REACTMOD_POS(i) (QString::fromUtf8(u8"反应模块%1提架").arg(i).toStdString())
#define STR_REACTMOD_PRE_POS(i) (QString::fromUtf8(u8"反应模块%1预放架").arg(i).toStdString())
#define STR_REACTMOD_PRE_PUT_HEIGHT(i) (QString::fromUtf8(u8"反应模块%1预放架高度").arg(i).toStdString())
#define STR_REACTMOD_PICK_HEIGHT(i) (QString::fromUtf8(u8"反应模块%1提架高度").arg(i).toStdString())
#define STR_REACTMOD_PUT_HEIGHT(i) (QString::fromUtf8(u8"反应模块%1放架高度").arg(i).toStdString())
#define STR_PIPET_HEIGHT(i) (QString::fromUtf8(u8"反应模块%1加液高度").arg(i).toStdString())
#define STR_REPAIRMOD_POS(i) (QString::fromUtf8(u8"修复模块提架%1").arg(i).toStdString())
#define STR_REPAIRMOD_HEIGHT(i) (QString::fromUtf8(u8"修复模块提架高度%1").arg(i).toStdString())
#define STR_REPAIRMOD_PUT_POS(i) (QString::fromUtf8(u8"修复模块放架高度%1").arg(i).toStdString())
#define STR_REACT_CLEAN_POS(i) (QString::fromUtf8(u8"清洗位置").toStdString())
#define STR_REACT_CLEAN_HEIGHT(i) (QString::fromUtf8(u8"清洗高度").toStdString())

//Add by ZhangWeiqi 
//Axis Name Define
#define STR_AXIS_NDL (u8"针头轴")
#define STR_AXIS_PIP (u8"移液轴")
#define STR_AXIS_X (u8"X轴")
#define STR_AXIS_Y (u8"Y轴")
#define STR_AXIS_PICK (u8"提架轴")

//Section Define
#define STR_SECTION_AXIS ("Axis")
#define STR_SECTION_RESOLUTION ("Resolution")
#define STR_SECTION_PUMPPARAM (u8"柱塞泵")

#define OPTYPE_CREAGENT (u8"通用试剂")
#define OPTYPE_VREAGENT (u8"可变试剂")
#define OPTYPE_WASH (u8"清洗")