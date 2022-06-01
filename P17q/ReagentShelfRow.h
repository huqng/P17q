#pragma once
#include <string>

#include "ui_ReagentShelfListRow.h"

using std::string;

class ReagentShelfRow : public QWidget
{
	Q_OBJECT
public:
	ReagentShelfRow(QWidget* parent, int iPos, bool bNonEmpty, const QString& strReagent = "", const QStringList& strlistAllReagent = {});
	void SetPos(int iPos);
	int GetPos()const;
	bool NonEmpty()const;
	string GetReagentName()const;
private:
	Ui::ReagentShelfListRowUi ui;
signals:
public slots:
	void onCheckBoxNonEmptyStateChanged(int);

};

