#include <qmessagebox.h>
#include <qvalidator.h>

#include "AddSlideDialog.h"

AddSlideDialog::AddSlideDialog(int iMaxCnt, QWidget *parent)
	: m_iMaxSlideCnt(iMaxCnt),
	QDialog(parent)
{
	ui.setupUi(this);

	ui.lineEditSlideCnt->setValidator(new QIntValidator(1, 10));
	connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &AddSlideDialog::onBtnAccepted);
}

AddSlideDialog::~AddSlideDialog()
{

}

QString AddSlideDialog::GetID()const
{
	return m_strID;
}

QString AddSlideDialog::GetHospital()const
{
	return m_strHospital;
}

int AddSlideDialog::GetSlideCnt()const
{
	return m_iSlideCnt;
}

void AddSlideDialog::onBtnAccepted()
{
	if (ui.lineEditID->text().isEmpty())
	{
		QMessageBox::warning(nullptr, QString::fromUtf8(u8""), QString::fromUtf8(u8"ID为空"));
		return;
	}
	else if (ui.lineEditHospital->text().isEmpty())
	{
		QMessageBox::warning(nullptr, QString::fromUtf8(u8""), QString::fromUtf8(u8"医院为空"));
		return;
	}
	else if (ui.lineEditSlideCnt->text().toInt() > m_iMaxSlideCnt)
	{
		QMessageBox::warning(nullptr, QString::fromUtf8(u8""), QString::fromUtf8(u8"玻片过多"));
		return;
	}
	else if (ui.lineEditSlideCnt->text().toInt() == 0)
	{
		QMessageBox::warning(nullptr, QString::fromUtf8(u8""), QString::fromUtf8(u8"没有玻片"));
		return;
	}
	else
	{
		m_strID = ui.lineEditID->text();
		m_strHospital = ui.lineEditHospital->text();
		m_iSlideCnt = ui.lineEditSlideCnt->text().toInt();
		this->accept();
	}
}
