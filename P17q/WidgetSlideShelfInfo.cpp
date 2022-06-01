#include "WidgetSlideShelfInfo.h"

static const QString s_strSlideQSSFormat = "QLabel { background: %1; border: 1px solid black; }";
static const vector<QString> s_vecColorNames = { "lightgray", "yellow", "green" ,"red" };

WidgetSlideShelfInfo::WidgetSlideShelfInfo(QWidget* parent) :
	QWidget(parent)
{
	ui.setupUi(this);
}

void WidgetSlideShelfInfo::Refresh(vector<int> vecColors)
{
	ui.lblPos1-> setStyleSheet(s_strSlideQSSFormat.arg(s_vecColorNames[vecColors[0]]));
	ui.lblPos2-> setStyleSheet(s_strSlideQSSFormat.arg(s_vecColorNames[vecColors[1]]));
	ui.lblPos3-> setStyleSheet(s_strSlideQSSFormat.arg(s_vecColorNames[vecColors[2]]));
	ui.lblPos4-> setStyleSheet(s_strSlideQSSFormat.arg(s_vecColorNames[vecColors[3]]));
	ui.lblPos5-> setStyleSheet(s_strSlideQSSFormat.arg(s_vecColorNames[vecColors[4]]));
	ui.lblPos6-> setStyleSheet(s_strSlideQSSFormat.arg(s_vecColorNames[vecColors[5]]));
	ui.lblPos7-> setStyleSheet(s_strSlideQSSFormat.arg(s_vecColorNames[vecColors[6]]));
	ui.lblPos8-> setStyleSheet(s_strSlideQSSFormat.arg(s_vecColorNames[vecColors[7]]));
	ui.lblPos9-> setStyleSheet(s_strSlideQSSFormat.arg(s_vecColorNames[vecColors[8]]));
	ui.lblPos10->setStyleSheet(s_strSlideQSSFormat.arg(s_vecColorNames[vecColors[9]]));	
}
