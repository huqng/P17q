#pragma once
#include "ui_WidgetSlideSHelfInfoUI.h"
#include <qwidget.h>
#include <qsettings.h>

using namespace std;

class WidgetSlideShelfInfo : public QWidget
{
	Q_OBJECT
public:
	WidgetSlideShelfInfo(QWidget* parent);

	void Refresh(vector<int> vecColors);
private:
	Ui::Form ui;

	
};