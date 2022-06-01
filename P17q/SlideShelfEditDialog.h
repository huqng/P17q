#pragma once

#include <QDialog>
#include "ui_SlideShelfEditDialog.h"

class SlideShelfEditDialog : public QDialog
{
	Q_OBJECT

public:
	SlideShelfEditDialog(QStringList strlistAllFormula, QStringList strlistAllReagent, QWidget *parent = Q_NULLPTR);
	~SlideShelfEditDialog();

	int GetSlideCnt()const;
	int GetRepairTime()const;
	int GetRepairTemp()const;
	QString GetFormula()const;
	QString GetSlideID(int iPos)const;
	QString GetSlideHospital(int iPos)const;
	int GetTotalStep()const;
	bool IsVReagent(int iStep)const;
	QString GetReagentName(int iStep, int iPos)const;

	void Save(const QString& strFilename);

protected:
	void InitTableRows(int iRowBegin, int iRowEnd);
	bool AllVReagentAssigned();

private:
	Ui::SlideShelfEditDialog ui;

	QStringList m_strlistAllFormula;
	QStringList m_strlistAllReagent;
public slots:
	void onBtnAddSlideClicked();
	void onBtnRemoveSlideClicked();
	void onBtnBoxAccepted();
	void onComboFormulaChanged(int);
	void onBtnImportClicked();
	void onBtnExportClicked();
};
