#include <boost/algorithm/string.hpp>

#include <qcombobox.h>
#include <qfiledialog.h>
#include <qmessagebox.h>

#include "../MachineControlQt/ini.h"
#include "def.h"

#include "SlideShelfEditDialog.h"

#include "AddSlideDialog.h"

SlideShelfEditDialog::SlideShelfEditDialog(QStringList strlistAllFormula, QStringList strlistAllReagent, QWidget *parent)
	: m_strlistAllFormula(strlistAllFormula),
	m_strlistAllReagent(strlistAllReagent),
	QDialog(parent)
{
	ui.setupUi(this);

	ui.lineEditRepairTemp->setValidator(new QIntValidator(0, 999));
	ui.lineEditRepairTime->setValidator(new QIntValidator(0, 99999));
	ui.lineEditRepairTemp->setText("95");
	ui.lineEditRepairTime->setText("1500");

	connect(ui.btnAddSlide, &QPushButton::clicked, this, &SlideShelfEditDialog::onBtnAddSlideClicked);
	connect(ui.btnRemoveSlide, &QPushButton::clicked, this, &SlideShelfEditDialog::onBtnRemoveSlideClicked);
	connect(ui.btnBox, &QDialogButtonBox::accepted, this, &SlideShelfEditDialog::onBtnBoxAccepted);
	connect(ui.comboBoxFormula, &QComboBox::currentIndexChanged, this, &SlideShelfEditDialog::onComboFormulaChanged);
	connect(ui.btnImport, &QPushButton::clicked, this, &SlideShelfEditDialog::onBtnImportClicked);
	connect(ui.btnExport, &QPushButton::clicked, this, &SlideShelfEditDialog::onBtnExportClicked);

	
	ui.comboBoxFormula->addItems(strlistAllFormula); /* add items after connection */
}

SlideShelfEditDialog::~SlideShelfEditDialog()
{

}

int SlideShelfEditDialog::GetSlideCnt()const
{
	return ui.tableWidget->rowCount();
}

int SlideShelfEditDialog::GetRepairTime()const
{
	return ui.lineEditRepairTime->text().toInt();
}

int SlideShelfEditDialog::GetRepairTemp()const
{
	return ui.lineEditRepairTemp->text().toInt();
}

QString SlideShelfEditDialog::GetSlideID(int iPos)const
{
	return dynamic_cast<QLabel*>(ui.tableWidget->cellWidget(iPos - 1, 0))->text();
}

QString SlideShelfEditDialog::GetSlideHospital(int iPos)const
{
	return dynamic_cast<QLabel*>(ui.tableWidget->cellWidget(iPos - 1, 1))->text();
}

int SlideShelfEditDialog::GetTotalStep()const
{
	return ui.tableWidget->columnCount() - 2;
}

bool SlideShelfEditDialog::IsVReagent(int iStep)const
{
	return ui.tableWidget->horizontalHeaderItem(2 + iStep)->text().contains(TYPE_VREAGENT);
}

QString SlideShelfEditDialog::GetReagentName(int iStep, int iPos)const
{
	if (this->IsVReagent(iStep))
	{
		return dynamic_cast<QComboBox*>(ui.tableWidget->cellWidget(iPos - 1, iStep + 2))->currentText();
	}
	else
	{
		return ui.tableWidget->item(iPos - 1, iStep + 2)->text();
	}
}

void SlideShelfEditDialog::Save(const QString& strFilename)
{
	mINI::INIStructure iniContent;

	/* set content by dlg */
	const int iTotalStep = ui.tableWidget->columnCount() - 2;
	const int iCnt = ui.tableWidget->rowCount();
	const int iRepairTime = ui.lineEditRepairTime->text().toInt();
	const int iRepairTemp = ui.lineEditRepairTemp->text().toInt();
	const QString strFormula = ui.comboBoxFormula->currentText();

	iniContent["Config"]["RepairTime"] = std::to_string(iRepairTime);
	iniContent["Config"]["RepairTemp"] = std::to_string(iRepairTemp);
	iniContent["Config"]["Formula"] = strFormula.toStdString();

	for (int iPos = 1; iPos <= iCnt; iPos++)
	{
		auto& iniSection = iniContent[std::to_string(iPos)];

		QString strID = this->GetSlideID(iPos);
		QString strHospital = this->GetSlideHospital(iPos);
		iniSection["ID"] = strID.toStdString();
		iniSection["Hospital"] = strHospital.toStdString();
		for (int iStep = 0; iStep < iTotalStep; iStep++)
		{
			if (this->IsVReagent(iStep))
			{
				iniSection[string("vReagent").append(std::to_string(iStep))] = this->GetReagentName(iStep, iPos).toStdString();
			}
		}
	}

	mINI::INIFile(strFilename.toLocal8Bit().toStdString()).write(iniContent);
}

QString SlideShelfEditDialog::GetFormula()const
{
	return ui.comboBoxFormula->currentText();
}

void SlideShelfEditDialog::InitTableRows(int iRowBegin, int iRowEnd)
{
	std::string strFormula = ui.comboBoxFormula->currentText().toStdString();
	mINI::INIStructure iniFormula;
	mINI::INIFile(std::string("./Formula/") + StrToHex(strFormula)).read(iniFormula);
	int iStepCnt = iniFormula[strFormula].size();
	for (int i = 0; i < iStepCnt; i++)
	{
		const int iCol = 2 + i;

		std::string strRow = iniFormula[strFormula][std::to_string(i + 1)];
		std::vector<std::string> vecSplit;
		boost::algorithm::split(vecSplit, strRow, boost::is_any_of(","));

		const std::string& strType = vecSplit.at(0);
		const std::string& strReagent = vecSplit.at(1);
		int iQuantity = std::stoi(vecSplit.at(2));
		int iTime = std::stoi(vecSplit.at(3));

		ui.tableWidget->setHorizontalHeaderItem(iCol, new QTableWidgetItem(U8_TO_QSTR(u8"步骤%1_").arg(1 + i) + U8_TO_QSTR(strType)));

		if (strType == QSTR_TO_U8(TYPE_CREAGENT))
		{
			for (int iRow = iRowBegin; iRow < iRowEnd; iRow++)
			{
				ui.tableWidget->setItem(iRow, iCol, new QTableWidgetItem(U8_TO_QSTR(strReagent)));
			}
		}
		else if (strType == QSTR_TO_U8(TYPE_VREAGENT))
		{
			for (int iRow = iRowBegin; iRow < iRowEnd; iRow++)
			{
				QComboBox* pComboReagent = new QComboBox;
				pComboReagent->addItems(m_strlistAllReagent);
				pComboReagent->setCurrentIndex(-1);
				ui.tableWidget->setCellWidget(iRow, iCol, pComboReagent);
			}
		}
		else if (strType == QSTR_TO_U8(TYPE_WASH))
		{
			for (int iRow = iRowBegin; iRow < iRowEnd; iRow++)
			{
				ui.tableWidget->setItem(iRow, iCol, new QTableWidgetItem(U8_TO_QSTR(u8"清洗")));
			}
		}
	}
}

bool SlideShelfEditDialog::AllVReagentAssigned()
{
	for (int iCol = 2; iCol < ui.tableWidget->columnCount(); iCol++)
	{
		QString strHeader = ui.tableWidget->horizontalHeaderItem(iCol)->text();
		if (strHeader.contains(TYPE_VREAGENT))
		{
			for (int iRow = 0; iRow < ui.tableWidget->rowCount(); iRow++)
			{
				if (dynamic_cast<QComboBox*>(ui.tableWidget->cellWidget(iRow, iCol))->currentIndex() < 0)
					return false;
			}
		}
	}
	return true;
}

void SlideShelfEditDialog::onBtnAddSlideClicked()
{
	AddSlideDialog dlg(10 - ui.tableWidget->rowCount());
	int iRet = dlg.exec();
	if (iRet == QDialog::Accepted)
	{
		int iNewSlideCnt = dlg.GetSlideCnt();
		int iPrevRowCount = ui.tableWidget->rowCount();
		ui.tableWidget->setRowCount(iPrevRowCount + iNewSlideCnt);

		QString strID = dlg.GetID();
		QString strHospital = dlg.GetHospital();

		for (int i = iPrevRowCount; i < iPrevRowCount + iNewSlideCnt; i++)
		{
			ui.tableWidget->setCellWidget(i, 0, new QLabel(strID/* + QString("_%1").arg(i - iPrevRowCount + 1)*/));
			ui.tableWidget->setCellWidget(i, 1, new QLabel(strHospital));
		}
		this->InitTableRows(iPrevRowCount, iPrevRowCount + iNewSlideCnt);
	}
}

void SlideShelfEditDialog::onBtnRemoveSlideClicked()
{
	int iCurRow = ui.tableWidget->currentRow();
	if (iCurRow >= 0)
	{
		ui.tableWidget->removeRow(iCurRow);
	}
	else
	{
		QMessageBox::warning(nullptr, QString::fromUtf8(u8""), QString::fromUtf8(u8"选中后可移除"));
	}
}

void SlideShelfEditDialog::onBtnBoxAccepted()
{
	if (ui.tableWidget->rowCount() <= 0)
	{
		QMessageBox::warning(nullptr, QString::fromUtf8(u8""), QString::fromUtf8(u8"没有玻片"));
	}
	else if (!AllVReagentAssigned())
	{
		QMessageBox::warning(nullptr, QString::fromUtf8(u8""), QString::fromUtf8(u8"没有指定试剂"));
	}
	else
	{
		this->accept();
	}
}

void SlideShelfEditDialog::onComboFormulaChanged(int)
{
	std::string strFormula = ui.comboBoxFormula->currentText().toStdString();
	mINI::INIStructure iniFormula;
	mINI::INIFile(std::string("./Formula/") + StrToHex(strFormula)).read(iniFormula);
	int iStepCnt = iniFormula[strFormula].size();
	ui.tableWidget->setColumnCount(2); /* clear prev formula */
	ui.tableWidget->setColumnCount(2 + iStepCnt);
	this->InitTableRows(0, ui.tableWidget->rowCount());
}

void SlideShelfEditDialog::onBtnImportClicked()
{
	QString strFilename = QFileDialog::getOpenFileName(nullptr, QString::fromStdString(u8"导入自"), "./Product/", "*.pro2");
	if (!strFilename.isEmpty())
	{
		mINI::INIStructure iniStruct;
		mINI::INIFile(strFilename.toLocal8Bit().toStdString()).read(iniStruct);
		const string strFormula = iniStruct["Config"]["Formula"];
		ui.comboBoxFormula->setCurrentText(QString::fromStdString(strFormula));
		ui.tableWidget->setRowCount(iniStruct.size() - 1);
		this->InitTableRows(0, ui.tableWidget->rowCount());

		const int iRepairTime = stoi(iniStruct["Config"]["RepairTime"]);
		const int iRepairTemp = stoi(iniStruct["Config"]["RepairTemp"]);
		ui.lineEditRepairTime->setText(QString::number(iRepairTime));
		ui.lineEditRepairTemp->setText(QString::number(iRepairTemp));

		for (int i = 0; i < iniStruct.size() - 1; i++)
		{
			const string& strID = iniStruct[std::to_string(i + 1)]["ID"];
			const string& strHospital = iniStruct[std::to_string(i + 1)]["Hospital"];
			ui.tableWidget->setCellWidget(i, 0, new QLabel(QString::fromStdString(strID)));
			ui.tableWidget->setCellWidget(i, 1, new QLabel(QString::fromStdString(strHospital)));


			for (int iCol = 2; iCol < ui.tableWidget->columnCount(); iCol++)
			{
				const QString strType = ui.tableWidget->horizontalHeaderItem(iCol)->text();
				if (strType.contains(TYPE_VREAGENT))
				{
					const string& strVReagent = iniStruct[std::to_string(i + 1)][string("vReagent").append(std::to_string(iCol - 2))];
					dynamic_cast<QComboBox*>(ui.tableWidget->cellWidget(i, iCol))->setCurrentText(QString::fromUtf8(strVReagent));
				}
			}
		}
	}
}

void SlideShelfEditDialog::onBtnExportClicked()
{
	QString strFilename = QFileDialog::getSaveFileName(nullptr, QString::fromStdString(u8"导出到"), "./Product/", "*.pro2");
	this->Save(strFilename);	
}
