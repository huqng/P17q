#include "def.h"
#include "ReagentShelfRow.h"

ReagentShelfRow::ReagentShelfRow(QWidget* parent, int iPos, bool bNonEmpty, const QString& strReagent, const QStringList& strlistAllReagent):
	QWidget(parent)
{
	ui.setupUi(this);

	ui.lblPos->setText(QString::number(iPos));
	ui.checkBoxNonEmpty->setChecked(bNonEmpty);
	ui.comboReagent->addItems(strlistAllReagent);
	if (bNonEmpty)
	{
		ui.comboReagent->setCurrentText(strReagent);
	}
	else
	{
		ui.comboReagent->setCurrentIndex(-1);
		ui.comboReagent->setDisabled(true);
	}

	connect(ui.checkBoxNonEmpty, &QCheckBox::stateChanged, this, &ReagentShelfRow::onCheckBoxNonEmptyStateChanged);
}

void ReagentShelfRow::SetPos(int iPos)
{
	ui.lblPos->setText(QString::number(iPos));
}

int ReagentShelfRow::GetPos()const
{
	return ui.lblPos->text().toInt();
}

bool ReagentShelfRow::NonEmpty()const
{
	return ui.checkBoxNonEmpty->isChecked();
}

string ReagentShelfRow::GetReagentName()const
{
	return QSTR_TO_U8(ui.comboReagent->currentText());
}

void ReagentShelfRow::onCheckBoxNonEmptyStateChanged(int iState)
{
	if (iState == Qt::CheckState::Checked)
	{
		ui.comboReagent->setEnabled(true);
	}
	else if(iState == Qt::CheckState::Unchecked)
	{
		ui.comboReagent->setDisabled(true);
	}
}
