#pragma once

#include <QDialog>
#include "ui_AddSlideDialog.h"

class AddSlideDialog : public QDialog
{
	Q_OBJECT

public:
	AddSlideDialog(int iMaxCnt, QWidget *parent = Q_NULLPTR);
	~AddSlideDialog();

	QString GetID()const;
	QString GetHospital()const;
	int GetSlideCnt()const;
private:
	Ui::AddSlideDialog ui;

	int m_iMaxSlideCnt;

	QString m_strID, m_strHospital;
	int m_iSlideCnt;

public slots:
	void onBtnAccepted();
};
