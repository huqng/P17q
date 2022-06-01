#pragma once
#include "ui_FormulaRow.h"

class FormulaRow: public QWidget
{
	Q_OBJECT
private:
	Ui::FormulaRowUI ui;
public:
	FormulaRow(QWidget* parent, int iIndex, QString strType,		
		QStringList strlistAllReagents, QString strReagent, 
		int iQuantity, int iTime);
	FormulaRow(QStringList strlistAllReagent, const FormulaRow&);


	bool CReagentAssigned()const;
	void SetIndex(int iIndex);
	QString GetRowText();
	void IncIndex();
	void DecIndex();

public slots:
	void onComboTypeChanged(int);
};

