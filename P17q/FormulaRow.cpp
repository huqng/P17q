#include "def.h"

#include "FormulaRow.h"

FormulaRow::FormulaRow(QWidget* parent, int iIndex, QString strType, QStringList strlistAllReagent, QString strReagent, int iQuantity, int iTime):
	QWidget(parent)	
{
	ui.setupUi(this);

	ui.lblIndex->setText(QString::number(iIndex));

	ui.comboBoxType->addItems(ALL_TYPE_LIST);
	ui.comboBoxType->setCurrentText(strType);
	ui.comboReagent->addItems(strlistAllReagent);
	if (strType != TYPE_CREAGENT)
	{
		ui.comboReagent->setCurrentIndex(-1);
		ui.comboReagent->setDisabled(true);
	}
	else
	{
		ui.comboReagent->setCurrentText(strReagent);
		ui.comboReagent->setEnabled(true);
	}

	ui.sbQuantity->setValue(iQuantity);
	ui.sbTime->setValue(iTime);

	connect(ui.comboBoxType, &QComboBox::currentIndexChanged, this, &FormulaRow::onComboTypeChanged);
}

FormulaRow::FormulaRow(QStringList strlistAllReagent, const FormulaRow& r):
	QWidget(r.parentWidget())
{
	ui.setupUi(this);
	ui.lblIndex->setText(r.ui.lblIndex->text());
	ui.comboBoxType->addItems(ALL_TYPE_LIST);
	ui.comboBoxType->setCurrentText(r.ui.comboBoxType->currentText());
	if (r.ui.comboBoxType->currentText() != TYPE_CREAGENT)
	{
		ui.comboReagent->setDisabled(true);
	}
	ui.comboReagent->addItems(strlistAllReagent);
	ui.comboReagent->setCurrentText(r.ui.comboReagent->currentText());
	ui.sbQuantity->setValue(r.ui.sbQuantity->value());
	ui.sbTime->setValue(r.ui.sbTime->value());
}

bool FormulaRow::CReagentAssigned()const
{
	if (ui.comboBoxType->currentText() == TYPE_CREAGENT)
		return ui.comboReagent->currentIndex() >= 0;
	else
		return true;
}

void FormulaRow::SetIndex(int iIndex)
{
	ui.lblIndex->setText(QString::number(iIndex));
}

QString FormulaRow::GetRowText()
{
	const QString strType = ui.comboBoxType->currentText();
	const QString strReagent = (strType == TYPE_CREAGENT) ? ui.comboReagent->currentText() : "wtf";
	const int iQuantity = ui.sbQuantity->value();
	const int iTime = ui.sbTime->value();

	return QString("%1,%2,%3,%4").arg(strType).arg(strReagent).arg(iQuantity).arg(iTime);
}

void FormulaRow::IncIndex()
{
	ui.lblIndex->setText(QString::number(ui.lblIndex->text().toInt() + 1));
}

void FormulaRow::DecIndex()
{
	ui.lblIndex->setText(QString::number(ui.lblIndex->text().toInt() - 1));
}

void FormulaRow::onComboTypeChanged(int)
{
	if (ui.comboBoxType->currentText() != TYPE_CREAGENT)
	{
		ui.comboReagent->setCurrentIndex(-1);
		ui.comboReagent->setDisabled(true);
	}
	else
	{
		ui.comboReagent->setEnabled(true);
	}
}
