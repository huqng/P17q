#include <boost/format.hpp>

#include <Windows.h>

#include <qevent.h>
#include <qdialog.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qinputdialog.h>

#include "ini.h"

#include "def.h"

#include "ReagentShelfRow.h"
#include "FormulaRow.h"
#include "SlideShelfEditDialog.h"

#include "MainWindow.h"

string StrToHex(string strIn)
{
	static boost::format fmt("%02X");
	string strHex;
	for (unsigned char uc : strIn)
	{
		strHex.append((fmt % int(uc)).str());
	}
	return strHex;
}

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	/* ui */
	ui.setupUi(this);
	/* home buttons */
	ui.btnStart->setDisabled(true);
	connect(ui.btnReset, &QPushButton::clicked, this, &MainWindow::onBtnResetClicked);
	connect(ui.btnStart, &QPushButton::clicked, this, &MainWindow::onBtnStartClicked);
	connect(ui.btnStop, &QPushButton::clicked, this, &MainWindow::onBtnStopClicked);
	connect(ui.btnTest, &QPushButton::clicked, this, &MainWindow::onBtnTestClicked);
	connect(ui.btnReload1, &QPushButton::clicked, this, &MainWindow::onBtnReload1Clicked);
	connect(ui.btnReload2, &QPushButton::clicked, this, &MainWindow::onBtnReload2Clicked);
	connect(ui.btnReload3, &QPushButton::clicked, this, &MainWindow::onBtnReload3Clicked);
	connect(ui.btnReload4, &QPushButton::clicked, this, &MainWindow::onBtnReload4Clicked);
	connect(ui.btnClearMsg, &QPushButton::clicked, this, &MainWindow::onBtnClearMsgClicked);
	/* product info */
	dynamic_cast<QLabel*>(ui.SlideShelf1->layout()->itemAt(0)->widget())->setText(U8_TO_QSTR(u8"玻片架1"));
	dynamic_cast<QLabel*>(ui.SlideShelf2->layout()->itemAt(0)->widget())->setText(U8_TO_QSTR(u8"玻片架2"));
	dynamic_cast<QLabel*>(ui.SlideShelf3->layout()->itemAt(0)->widget())->setText(U8_TO_QSTR(u8"玻片架3"));
	dynamic_cast<QLabel*>(ui.SlideShelf4->layout()->itemAt(0)->widget())->setText(U8_TO_QSTR(u8"玻片架4"));
	dynamic_cast<QLabel*>(ui.TransitShelf->layout()->itemAt(0)->widget())->setText(U8_TO_QSTR(u8"中转"));
	dynamic_cast<QLabel*>(ui.RepairModule1->layout()->itemAt(0)->widget())->setText(U8_TO_QSTR(u8"修复模块1"));
	dynamic_cast<QLabel*>(ui.RepairModule2->layout()->itemAt(0)->widget())->setText(U8_TO_QSTR(u8"修复模块2"));
	/* config */
	connect(ui.comboConfigGroup, &QComboBox::currentTextChanged, this, &MainWindow::slotComboConfigTextChanged);
	connect(ui.comboCoordType, &QComboBox::currentTextChanged, this, &MainWindow::slotComboCoordTypeTextChanged);
	connect(ui.comboCoordName, &QComboBox::currentTextChanged, this, &MainWindow::slotComboCoordNameTextChanged);
	connect(ui.btnSaveConfig, &QPushButton::clicked, this, &MainWindow::onBtnSaveConfigClicked);
	connect(ui.btnSaveCoord, &QPushButton::clicked, this, &MainWindow::onBtnSaveCoordClicked);
	connect(ui.btnMoveToCoord, &QPushButton::clicked, this, &MainWindow::slotBtnMoveToCoordClicked);
	connect(ui.btnMoveNegtive1, &QPushButton::clicked, this, &MainWindow::onBtnCoordMoveNegtiveX);
	connect(ui.btnMovePositive1, &QPushButton::clicked, this, &MainWindow::onBtnCoordMovePositiveX);
	connect(ui.btnMoveNegtive2, &QPushButton::clicked, this, &MainWindow::onBtnCoordMoveNegtiveY);
	connect(ui.btnMovePositive2, &QPushButton::clicked, this, &MainWindow::onBtnCoordMovePositiveY);
	connect(ui.btnMoveNegtive3, &QPushButton::clicked, this, &MainWindow::onBtnCoordMoveNegtiveZ1);
	connect(ui.btnMovePositive3, &QPushButton::clicked, this, &MainWindow::onBtnCoordMovePositiveZ1);
	connect(ui.btnMoveNegtive4, &QPushButton::clicked, this, &MainWindow::onBtnCoordMoveNegtiveZ2);
	connect(ui.btnMovePositive4, &QPushButton::clicked, this, &MainWindow::onBtnCoordMovePositiveZ2);
	connect(ui.btnMoveNegtive5, &QPushButton::clicked, this, &MainWindow::onBtnCoordMoveNegtiveZ3);
	connect(ui.btnMovePositive5, &QPushButton::clicked, this, &MainWindow::onBtnCoordMovePositiveZ3);
	connect(ui.btnSingleAxisHome1, &QPushButton::clicked, this, &MainWindow::onBtnSingleAxisHome1Clicked);
	connect(ui.btnSingleAxisHome2, &QPushButton::clicked, this, &MainWindow::onBtnSingleAxisHome2Clicked);
	connect(ui.btnSingleAxisHome3, &QPushButton::clicked, this, &MainWindow::onBtnSingleAxisHome3Clicked);
	connect(ui.btnSingleAxisHome4, &QPushButton::clicked, this, &MainWindow::onBtnSingleAxisHome4Clicked);
	connect(ui.btnSingleAxisHome5, &QPushButton::clicked, this, &MainWindow::onBtnSingleAxisHome5Clicked);
	connect(ui.btnEmgStop, &QPushButton::clicked, this, &MainWindow::onBtnEmgStopClicked);
	connect(ui.btnZAxesUp, &QPushButton::clicked, this, &MainWindow::onBtnZAxisUpClicked);
	connect(ui.cbIO0, &QCheckBox::clicked, this, &MainWindow::onCheckBoxIO0Clicked);
	connect(ui.cbIO1, &QCheckBox::clicked, this, &MainWindow::onCheckBoxIO1Clicked);
	connect(ui.cbIO2, &QCheckBox::clicked, this, &MainWindow::onCheckBoxIO2Clicked);
	connect(ui.btnCalcReagentPos, &QPushButton::clicked, this, &MainWindow::onBtnCalcReagentPosClicked);
	connect(ui.btnCalcSlidePos1, &QPushButton::clicked, this, &MainWindow::onBtnCalcSlidePos1Clicked);
	connect(ui.btnCalcSlidePos2, &QPushButton::clicked, this, &MainWindow::onBtnCalcSlidePos2Clicked);
	ui.lineEditDTVal->setValidator(new QIntValidator(-0x80000000, 0x7FFFFFFF));
	ui.lineEditRelayVal->setValidator(new QIntValidator(0, 1));
	connect(ui.btnWriteDT, &QPushButton::clicked, this, &MainWindow::onBtnWriteDTClicked);
	connect(ui.btnSetRelay, &QPushButton::clicked, this, &MainWindow::onBtnSetRelayClicked);
	connect(ui.btnClearRelay, &QPushButton::clicked, this, &MainWindow::onBtnClearRelayClicked);
	connect(ui.btnReadDT, &QPushButton::clicked, this, &MainWindow::onBtnReadDTClicked);
	connect(ui.btnReadRelay, &QPushButton::clicked, this, &MainWindow::onBtnReadRelayClicked);
	connect(ui.comboBoxPLCStation, &QComboBox::currentTextChanged, this, &MainWindow::onComboPLCStationTextChanged);
	connect(ui.comboBoxDTAddrs, &QComboBox::currentTextChanged, this, [this] {ui.lineEditDTVal->clear(); });
	connect(ui.comboBoxRelayAddrs, &QComboBox::currentTextChanged, this, [this] {ui.lineEditRelayVal->clear(); });

	/* reagents */
	connect(ui.listAllReagents, &QListWidget::currentItemChanged, this, &MainWindow::onListAllReagentsCurrentItemChanged);
	connect(ui.btnAddReagent, &QPushButton::clicked, this, &MainWindow::onBtnAddReagentClicked);
	connect(ui.btnRemoveReagent, &QPushButton::clicked, this, &MainWindow::onBtnRemoveReagentClicked);
	connect(ui.btnMoveUpReagent, &QPushButton::clicked, this, &MainWindow::onBtnMoveUpReagentClicked);
	connect(ui.btnMoveDownReagent, &QPushButton::clicked, this, &MainWindow::onBtnMoveDownReagentClicked);
	connect(ui.btnSaveReagent, &QPushButton::clicked, this, &MainWindow::onBtnSaveReagentClicked);
	/* reagent shelf editor */
	connect(ui.comboReagentShelves, &QComboBox::currentTextChanged, this, &MainWindow::onComboReagentShelfChanged);
	connect(ui.btnSaveReagentShelf, &QPushButton::clicked, this, &MainWindow::onBtnSaveReagentShelfClicked);
	/* formula editor */
	connect(ui.comboBoxFormula, &QComboBox::currentIndexChanged, this, &MainWindow::onComboFormulaChanged);
	connect(ui.listFormulaDetail, &QListWidget::currentItemChanged, this, &MainWindow::onListFormulaDetailCurrentItemChanged);
	connect(ui.btnAddFormulaRow, &QPushButton::clicked, this, &MainWindow::onBtnAddFormulaRowClicked);
	connect(ui.btnRemoveFormulaRow, &QPushButton::clicked, this, &MainWindow::onBtnRemoveFormulaRowClicked);
	connect(ui.btnMoveupFormulaRow, &QPushButton::clicked, this, &MainWindow::onBtnMoveUpFormulaRowClicked);
	connect(ui.btnMovedownFormulaRow, &QPushButton::clicked, this, &MainWindow::onBtnMoveDownFormulaRowClicked);
	connect(ui.btnCreateFormula, &QPushButton::clicked, this, &MainWindow::onBtnCreateFormulaClicked);
	connect(ui.btnRemoveFormula, &QPushButton::clicked, this, &MainWindow::onBtnDeleteFormulaClicked);
	connect(ui.btnSaveFormula, &QPushButton::clicked, this, &MainWindow::onBtnSaveFormulaClicked);

	/* machine-control object */
	m_pControl = GetMachineControlInstance();
	connect(m_pControl, &CIMachineControl::signalResetFinished, this, &MainWindow::slotResetFinished);
	connect(m_pControl, &CIMachineControl::signalUpdateAllView, this, &MainWindow::slotUpdateHomeView);
	connect(m_pControl, &CIMachineControl::signalUpdateReactionModuleView, this, &MainWindow::slotUpdateReactModView);
	connect(m_pControl, &CIMachineControl::signalUpdateRepairModuleView, this, &MainWindow::slotUpdateRepairModView);
	connect(m_pControl, &CIMachineControl::signalUpdateShelfHolderView, this, &MainWindow::slotUpdateShelfHolderView);
	connect(m_pControl, &CIMachineControl::signalUpdateTransitShelfView, this, &MainWindow::slotUpdateTransitShelfView);

	connect(m_pControl, &CIMachineControl::signalInterrupt, this, &MainWindow::slotInterrupt);
	connect(m_pControl, &CIMachineControl::signalMsg, this, &MainWindow::slotMsg);
	connect(m_pControl, &CIMachineControl::signalQuestion, this, &MainWindow::slotQuestion);
	connect(m_pControl, &CIMachineControl::signalRepairModAlarm, this, &MainWindow::slotRepairModAlarm);
	connect(m_pControl, &CIMachineControl::signalRepairModAlarm2, this, &MainWindow::slotRepairModAlarm2);
	connect(m_pControl, &CIMachineControl::signalUpdateReactModTem, this, &MainWindow::slotUpdateReactModTemperateView);
	connect(m_pControl, &CIMachineControl::signalUpdateRepairModTem, this, &MainWindow::slotUpdateRepairModTemperateView);
	connect(m_pControl, &CIMachineControl::signalUpdateCoord, this, &MainWindow::slotUpdateCoord);

	ReloadConfigView();
	ReloadCoordView();
	ReloadReagentView();
	ReloadFormulaView();
	//ReloadFormulaView();
	ReloadReagentShelfView();
	slotUpdateHomeView();
}

MainWindow::~MainWindow()
{
	m_pControl->Exit();
}

void MainWindow::closeEvent(QCloseEvent* evnt)
{
	int iRet = QMessageBox::question(nullptr, U8_TO_QSTR(u8"P17+"), U8_TO_QSTR(u8"关闭软件？"));
	if (iRet == QMessageBox::Yes)
	{
		evnt->accept();
	}
	else
	{
		evnt->ignore();
	}
}

void MainWindow::onBtnResetClicked()
{
	m_pControl->Reset();
	if (m_pControl->IsResetting())
	{
		ui.btnReset->setDisabled(true);
		ui.btnStart->setDisabled(true);
	}
}

void MainWindow::onBtnStartClicked()
{
	if (!m_pControl->IsRunning()) /* if not running, start */
	{
		if (m_pControl->IsStopped())
			m_pControl->Start();
		else
			m_pControl->Resume();

		if (m_pControl->IsRunning())
			ui.btnStart->setText(U8_TO_QSTR(u8"暂停"));
	}
	else /* if running, suspend */
	{
		m_pControl->Suspend();
		if (m_pControl->IsSuspend())
			ui.btnStart->setText(U8_TO_QSTR(u8"继续"));
	}
	ui.btnReset->setDisabled(true);
}

void MainWindow::onBtnStopClicked()
{
	m_pControl->Abort();
	if (m_pControl->IsStopped())
	{
		ui.btnStart->setDisabled(true);
		ui.btnReset->setEnabled(true);
	}
}

void MainWindow::onBtnTestClicked()
{
	this->onBtnStopClicked();

	static bool s_bTesting = false;
	if (s_bTesting)
	{
		s_bTesting = false;
		ui.btnTest->setText("Test");
		m_pControl->Test(0);
	}
	else
	{
		s_bTesting = true;
		m_pControl->Test(1);
		ui.btnTest->setText("Testing");
	}
}

void MainWindow::onBtnReload1Clicked()
{
	MachineReloadShelf(1);
	slotUpdateShelfHolderView(1);
}

void MainWindow::onBtnReload2Clicked()
{
	MachineReloadShelf(2);
	slotUpdateShelfHolderView(2);
}

void MainWindow::onBtnReload3Clicked() {
	MachineReloadShelf(3);
	slotUpdateShelfHolderView(3);
}

void MainWindow::onBtnReload4Clicked()
{
	MachineReloadShelf(4);
	slotUpdateShelfHolderView(4);
}

void MainWindow::onBtnClearMsgClicked()
{
	ui.listWidgetMsg->clear();
	void LogMsg();
}

/* save ini: update comboboxdata，write file，Control reload ini */

void MainWindow::onBtnSaveConfigClicked()
{
	int iRet = QMessageBox::question(nullptr, U8_TO_QSTR(u8"保存"), U8_TO_QSTR(u8"保存参数？"), QMessageBox::Ok, QMessageBox::Cancel);
	if (iRet == QMessageBox::Cancel) {
		return;
	}

	mINI::INIFile ini("./Config/Config.ini");
	mINI::INIStructure config;

	/* save current table into combodata */
	QMap<QString, QString> mapCurConfigGroup;
	for (int j = 0; j < ui.tblConfigContents->rowCount(); j++)
	{
		QString strKey = ui.tblConfigContents->item(j, 0)->text();
		QString strValue = ui.tblConfigContents->item(j, 1)->text();
		mapCurConfigGroup[strKey] = strValue;
	}
	ui.comboConfigGroup->setItemData(ui.comboConfigGroup->currentIndex(), QVariant::fromValue(mapCurConfigGroup)); /* re-select this combobox item */

	/* write all combodata into file */
	for (int i = 0; i < ui.comboConfigGroup->count(); i++)
	{
		string strConfigGroup = QSTR_TO_U8(ui.comboConfigGroup->itemText(i));
		QMap<QString, QString> mapConfigContent = ui.comboConfigGroup->itemData(i).value<QMap<QString, QString>>();
		for (const QString& strKey : mapConfigContent.keys())
		{                       
			config[strConfigGroup][QSTR_TO_U8(strKey)] = QSTR_TO_U8(mapConfigContent[strKey]);
		}
	}
	ini.write(config); 

	m_pControl->ReloadConfig();

	QMessageBox::information(this, "", U8_TO_QSTR(u8"已保存"));
}

void MainWindow::onBtnSaveCoordClicked()
{
	int iRet = QMessageBox::question(nullptr, U8_TO_QSTR(u8"保存"), U8_TO_QSTR(u8"保存坐标？"), QMessageBox::Ok, QMessageBox::Cancel);
	if (iRet == QMessageBox::Cancel) {
		return;
	}

	mINI::INIFile file("./Config/Position.pt");
	mINI::INIStructure coord;

	/* coord name -> axes bitmap, position */
	QMap<QString, QPair<int, QVector<double>>> mapCurCoords = ui.comboCoordType->currentData().value<QMap<QString, QPair<int, QVector<double>>>>();
	QString strCurCoordName = ui.comboCoordName->currentText();
	const int iCurAxisBitmap = mapCurCoords[strCurCoordName].first;
	QVector<double> vecCurCoordValue = { 
		ui.lblCurPos1->text().toDouble(), 
		ui.lblCurPos2->text().toDouble(), 
		ui.lblCurPos3->text().toDouble(), 
		ui.lblCurPos4->text().toDouble(), 
		ui.lblCurPos5->text().toDouble() };
	mapCurCoords[strCurCoordName].second = vecCurCoordValue;

	/* update data of comboCoordType */
	ui.comboCoordType->setItemData(ui.comboCoordType->currentIndex(), QVariant::fromValue(mapCurCoords));
	ui.comboCoordName->setCurrentText(strCurCoordName);

	for (int i = 0; i < ui.comboCoordType->count(); i++)
	{
		QMap<QString, QPair<int, QVector<double>>> mapData = ui.comboCoordType->itemData(i).value<QMap<QString, QPair<int, QVector<double>>>>();
		for (const QString& strCoordName : mapData.keys())
		{
			const QPair<int, QVector<double>>& vecCoord = mapData[strCoordName];
			coord[QSTR_TO_U8(strCoordName)]["AxisNos"] = to_string(vecCoord.first);
			for (int j = 0; j < 5; j++)
			{
				if (vecCoord.first & (1 << j))
				{
					coord[QSTR_TO_U8(strCoordName)][to_string(j)] = to_string(vecCoord.second[j]);
				}
			}
		}
	}
	file.write(coord);
	m_pControl->ReloadCoord();

	QMessageBox::information(this, "", U8_TO_QSTR(u8"已保存"));
}

void MainWindow::onBtnCoordMoveNegtiveX()
{
	if (m_pControl->MoveAxis(0, -GetMoveLength(), false) != 0)
	{
		QMessageBox::information(this, U8_TO_QSTR(u8"提示"), U8_TO_QSTR(u8"移动失败"));
	}
}

void MainWindow::onBtnCoordMovePositiveX()
{
	if (m_pControl->MoveAxis(0, GetMoveLength(), false) != 0)
	{
		QMessageBox::information(this, U8_TO_QSTR(u8"提示"), U8_TO_QSTR(u8"移动失败"));
	}
}

void MainWindow::onBtnCoordMoveNegtiveY()
{
	if (m_pControl->MoveAxis(1, -GetMoveLength(), false) != 0)
	{
		QMessageBox::information(this, U8_TO_QSTR(u8"提示"), U8_TO_QSTR(u8"移动失败"));
	}
}

void MainWindow::onBtnCoordMovePositiveY()
{
	if (m_pControl->MoveAxis(1, GetMoveLength(), false) != 0)
	{
		QMessageBox::information(this, U8_TO_QSTR(u8"提示"), U8_TO_QSTR(u8"移动失败"));
	}
}

void MainWindow::onBtnCoordMoveNegtiveZ1()
{
	if (m_pControl->MoveAxis(2, -GetMoveLength(), false) != 0)
	{
		QMessageBox::information(this, U8_TO_QSTR(u8"提示"), U8_TO_QSTR(u8"移动失败"));
	}
}

void MainWindow::onBtnCoordMovePositiveZ1()
{
	if (m_pControl->MoveAxis(2, GetMoveLength(), false) != 0)
	{
		QMessageBox::information(this, U8_TO_QSTR(u8"提示"), U8_TO_QSTR(u8"移动失败"));
	}
}

void MainWindow::onBtnCoordMoveNegtiveZ2()
{
	if (m_pControl->MoveAxis(3, -GetMoveLength(), false) != 0)
	{
		QMessageBox::information(this, U8_TO_QSTR(u8"提示"), U8_TO_QSTR(u8"移动失败"));
	}
}

void MainWindow::onBtnCoordMovePositiveZ2()
{
	if (m_pControl->MoveAxis(3, GetMoveLength(), false) != 0)
	{
		QMessageBox::information(this, U8_TO_QSTR(u8"提示"), U8_TO_QSTR(u8"移动失败"));
	}
}

void MainWindow::onBtnCoordMoveNegtiveZ3()
{
	if (m_pControl->MoveAxis(4, -GetMoveLength(), false) != 0)
	{
		QMessageBox::information(this, U8_TO_QSTR(u8"提示"), U8_TO_QSTR(u8"移动失败"));
	}
}

void MainWindow::onBtnCoordMovePositiveZ3()
{
	if (m_pControl->MoveAxis(4, GetMoveLength(), false) != 0)
	{
		QMessageBox::information(this, U8_TO_QSTR(u8"提示"), U8_TO_QSTR(u8"移动失败"));
	}
}

void MainWindow::onBtnSingleAxisHome1Clicked()
{
	if (m_pControl->HomeMoveAxis(0) != 0)
	{
		QMessageBox::information(this, U8_TO_QSTR(u8"提示"), U8_TO_QSTR(u8"移动失败"));
	}
}

void MainWindow::onBtnSingleAxisHome2Clicked()
{
	if (m_pControl->HomeMoveAxis(1) != 0)
	{
		QMessageBox::information(this, U8_TO_QSTR(u8"提示"), U8_TO_QSTR(u8"移动失败"));
	}
}

void MainWindow::onBtnSingleAxisHome3Clicked()
{
	if (m_pControl->HomeMoveAxis(2) != 0)
	{
		QMessageBox::information(this, U8_TO_QSTR(u8"提示"), U8_TO_QSTR(u8"移动失败"));
	}
}

void MainWindow::onBtnSingleAxisHome4Clicked()
{
	if (m_pControl->HomeMoveAxis(3) != 0)
	{
		QMessageBox::information(this, U8_TO_QSTR(u8"提示"), U8_TO_QSTR(u8"移动失败"));
	}
}

void MainWindow::onBtnSingleAxisHome5Clicked()
{
	if (m_pControl->HomeMoveAxis(4) != 0)
	{
		QMessageBox::information(this, U8_TO_QSTR(u8"提示"), U8_TO_QSTR(u8"移动失败"));
	}
}


void MainWindow::onBtnEmgStopClicked()
{
	m_pControl->Abort();
}

void MainWindow::onBtnZAxisUpClicked()
{
	if (m_pControl->ZAxisUp() != 0)
	{
		QMessageBox::information(this, U8_TO_QSTR(u8"提示"), U8_TO_QSTR(u8"移动失败"));
	}
}

void MainWindow::slotUpdateCoord(double, double, double, double, double)
{
	ui.lblCurPos1->setText(QString::number(m_pControl->GetAxisPos(0)));
	ui.lblCurPos2->setText(QString::number(m_pControl->GetAxisPos(1)));
	ui.lblCurPos3->setText(QString::number(m_pControl->GetAxisPos(2)));
	ui.lblCurPos4->setText(QString::number(m_pControl->GetAxisPos(3)));
	ui.lblCurPos5->setText(QString::number(m_pControl->GetAxisPos(4)));	
}

void MainWindow::onCheckBoxIO0Clicked(bool bChecked)
{
	m_pControl->SetNeedleCleanerPumpOn(bChecked);
}

void MainWindow::onCheckBoxIO1Clicked(bool bChecked)
{

}

void MainWindow::onCheckBoxIO2Clicked(bool bChecked)
{
	m_pControl->SetElectroMagnet(bChecked);
}

void MainWindow::onBtnCalcReagentPosClicked()
{
	mINI::INIFile file("./Config/Position.pt");
	mINI::INIStructure data;
	file.read(data);
	double dX1 = stod(data[string(u8"试剂架试剂1")]["0"]);
	double dY1 = stod(data[string(u8"试剂架试剂1")]["1"]);
	double dX6 = stod(data[string(u8"试剂架试剂6")]["0"]);
	double dY6 = stod(data[string(u8"试剂架试剂6")]["1"]);
	double dX13 = stod(data[string(u8"试剂架试剂13")]["0"]);
	double dY13 = stod(data[string(u8"试剂架试剂13")]["1"]);



	for (int i = 1; i <= 18; i++)
	{
		int iDX = (i - 1) / 6;
		int iDY = (i - 1) % 6;
		double dX = dX1 + (dX13 - dX1) * iDX / 2;
		double dY = dY1 + (dY6 - dY1) * iDY / 5;
		data[string(u8"试剂架试剂").append(to_string(i))]["0"] = to_string(dX);
		data[string(u8"试剂架试剂").append(to_string(i))]["1"] = to_string(dY);
	}

	file.write(data);
	QString strCurCoordType = ui.comboCoordType->currentText();
	QString strCurCoordName = ui.comboCoordName->currentText();
	this->ReloadCoordView();
	ui.comboCoordType->setCurrentText(strCurCoordType);
	ui.comboCoordName->setCurrentText(strCurCoordName);
	m_pControl->ReloadConfig();
}

void MainWindow::onBtnCalcSlidePos1Clicked()
{
	this->ImplCalcSlidePos(1);
}

void MainWindow::onBtnCalcSlidePos2Clicked()
{
	this->ImplCalcSlidePos(2);
}

void MainWindow::onBtnWriteDTClicked()
{
	int iDevID = ui.comboBoxPLCStation->currentData().value<QPair<int, QMap<QString, QMap<QString, QString>>>>().first;
	double dVal = ui.lineEditDTVal->text().toDouble();
	string strAddr = QSTR_TO_U8(ui.comboBoxDTAddrs->currentData().toString());
	m_pControl->WriteDT(iDevID, QSTR_TO_U8(ui.comboBoxDTAddrs->currentText()).c_str(), dVal);

	onBtnReadDTClicked();
}

void MainWindow::onBtnReadDTClicked()
{
	int iDevID = ui.comboBoxPLCStation->currentData().value<QPair<int, QMap<QString, QMap<QString, QString>>>>().first;
	string strAddr = QSTR_TO_U8(ui.comboBoxDTAddrs->currentData().toString());

	int iVal = m_pControl->ReadDT(iDevID, QSTR_TO_U8(ui.comboBoxDTAddrs->currentText()).c_str());
	ui.lineEditDTVal->setText(QString::number(iVal));
}

void MainWindow::onBtnSetRelayClicked()
{
	int iDevID = ui.comboBoxPLCStation->currentData().value<QPair<int, QMap<QString, QMap<QString, QString>>>>().first;
	string strAddr = QSTR_TO_U8(ui.comboBoxRelayAddrs->currentData().toString());
	m_pControl->WriteRelay(iDevID, strAddr.c_str(), true);

	onBtnReadRelayClicked();
}

void MainWindow::onBtnClearRelayClicked()
{
	int iDevID = ui.comboBoxPLCStation->currentData().value<QPair<int, QMap<QString, QMap<QString, QString>>>>().first;
	string strAddr = QSTR_TO_U8(ui.comboBoxRelayAddrs->currentData().toString());
	m_pControl->WriteRelay(iDevID, strAddr.c_str(), false);

	onBtnReadRelayClicked();
}

void MainWindow::onBtnReadRelayClicked()
{
	int iDevID = ui.comboBoxPLCStation->currentData().value<QPair<int, QMap<QString, QMap<QString, QString>>>>().first;
	string strAddr = QSTR_TO_U8(ui.comboBoxRelayAddrs->currentData().toString());

	bool bVal = m_pControl->ReadRelay(iDevID, strAddr.c_str());
	ui.lineEditRelayVal->setText(QString::number(bVal));
}

void MainWindow::onComboPLCStationTextChanged(const QString&)
{
	ui.comboBoxDTAddrs->clear();
	ui.comboBoxRelayAddrs->clear();
	QPair<int, QMap<QString, QMap<QString, QString>>> pairCurrentData = ui.comboBoxPLCStation->currentData().value<QPair<int, QMap<QString, QMap<QString, QString>>>>();
	const QMap<QString, QMap<QString, QString>>& mapCurrentData = pairCurrentData.second;
	 
	for (QString strSectionName : mapCurrentData.keys())
	{
		if (strSectionName.contains("DT"))
		{
			for (const QString& strAddrName : mapCurrentData[strSectionName].keys())
			{
				ui.comboBoxDTAddrs->addItem(strAddrName, QVariant::fromValue<QString>(mapCurrentData[strSectionName][strAddrName]));
			}
		}
		else if (strSectionName.contains("Relay"))
		{
			for (const QString& strAddrName : mapCurrentData[strSectionName].keys())
			{
				ui.comboBoxRelayAddrs->addItem(strAddrName, QVariant::fromValue<QString>(mapCurrentData[strSectionName][strAddrName]));
			}
		}
	}
}

double MainWindow::GetMoveLength()
{
	if (ui.rbtnStepLength01->isChecked())
		return 0.1;
	else if (ui.rbtnStepLength02->isChecked())
		return 0.2;
	else if (ui.rbtnStepLength05->isChecked())
		return 0.5;
	else if (ui.rbtnStepLength1->isChecked())
		return 1;
	else if (ui.rbtnStepLength5->isChecked())
		return 5;
	else if (ui.rbtnStepLength10->isChecked())
		return 10;
	else if (ui.rbtnStepLength20->isChecked())
		return 20;
	else if (ui.rbtnStepLength50->isChecked())
		return 50;
	else
		return 1;
}

void MainWindow::ImplCalcSlidePos(int iReactModIndex)
{
	int iIndex = iReactModIndex;
	mINI::INIFile file("./Config/Position.pt");
	mINI::INIStructure coord;
	file.read(coord);
	double dXMin = stod(coord[string(u8"反应模块").append(to_string(iIndex).append(u8"玻片1"))]["0"]);
	double dXMax = stod(coord[string(u8"反应模块").append(to_string(iIndex).append(u8"玻片10"))]["0"]);
	double dYMin = stod(coord[string(u8"反应模块").append(to_string(iIndex).append(u8"玻片1"))]["1"]);
	double dYMax = stod(coord[string(u8"反应模块").append(to_string(iIndex).append(u8"玻片10"))]["1"]);

	for (int i = 2; i <= 9; i++)
	{
		string strCoordName = u8"反应模块";
		strCoordName.append(to_string(iIndex)).append(u8"玻片").append(to_string(i));
		double dYVal = (dYMax - dYMin) * (i - 1) / 9 + dYMin;
		double dXVal = (dXMax - dXMin) * (i - 1) / 9 + dXMin;
		coord[strCoordName]["1"] = to_string(dYVal);
		coord[strCoordName]["0"] = to_string(dXVal);
	}
	file.write(coord);

	QString strCurCoordType = ui.comboCoordType->currentText();
	QString strCurCoordName = ui.comboCoordName->currentText();

	this->ReloadCoordView();

	ui.comboCoordType->setCurrentText(strCurCoordType);
	ui.comboCoordName->setCurrentText(strCurCoordName);


	m_pControl->ReloadConfig();
}

QStringList MainWindow::GetAllReagent()
{
	QStringList strlistAllReagent;
	for (int i = 0; i < ui.listAllReagents->count(); i++)
	{
		strlistAllReagent.append(ui.listAllReagents->item(i)->text());
	}
	return strlistAllReagent;
}

QStringList MainWindow::GetAllFormula()
{
	QStringList strlistAllFormula;
	
	for (int i = 0; i < ui.comboBoxFormula->count(); i++)
	{
		strlistAllFormula.append(ui.comboBoxFormula->itemText(i));
	}
	return strlistAllFormula;
}

void MainWindow::LogMsg(QString str)
{
	ofstream fout(string("./Log/").append(QDateTime::currentDateTime().toString("yyyy-MM-dd--hh").toStdString()).append(".log"), ios::out | ios::app);
	
	fout << str.toStdString() << endl;
	
}

void MainWindow::slotResetFinished()
{
	ui.btnReset->setEnabled(true);
	ui.btnStart->setEnabled(true); 
	ui.btnStart->setText(U8_TO_QSTR(u8"开始"));
}

void MainWindow::slotUpdateHomeView()
{
	slotUpdateReactModView(1);
	slotUpdateReactModView(2);
	slotUpdateRepairModView(1);
	slotUpdateRepairModView(2);
	slotUpdateShelfHolderView(1);
	slotUpdateShelfHolderView(2);
	slotUpdateShelfHolderView(3);
	slotUpdateShelfHolderView(4);
	slotUpdateTransitShelfView();
}

void MainWindow::slotUpdateReactModView(int iIndex)
{
	QVBoxLayout* pVLayoutReactMod = iIndex == 1 ? ui.vLayoutReactMod1 : pVLayoutReactMod = ui.vLayoutReactMod2;
	for (int j = 1; j <= 10; j++)
	{
		QProgressBar* pProgress = dynamic_cast<QProgressBar*>(pVLayoutReactMod->itemAt(j - 1)->widget());
		if (!m_pControl->HasSlideAtReactMod(iIndex, j))
		{
			pProgress->setMaximum(1);
			pProgress->setValue(0);
			pProgress->setDisabled(true);
			pProgress->setTextVisible(false);
		}
		else
		{
			int iReactionTotalStep1 = m_pControl->GetReactionTotalStep(iIndex, j);
			pProgress->setEnabled(true);
			pProgress->setMaximum(iReactionTotalStep1);
			pProgress->setValue(m_pControl->GetReactionCompletedStep(iIndex, j));
			pProgress->setTextVisible(true);
		}
	}
}

void MainWindow::slotUpdateRepairModView(int iIndex)
{
	vector<int> vecState(10);
	WidgetSlideShelfInfo* pWidget = iIndex == 1 ? ui.RepairModule1 : ui.RepairModule2;
	for (int j = 1; j <= 10; j++)
	{
		vecState[j - 1] = m_pControl->GetRepairModState(iIndex, j);
	}
	pWidget->Refresh(vecState);        
}

void MainWindow::slotUpdateShelfHolderView(int iIndex)
{
	vector<int> vecState(10);
	WidgetSlideShelfInfo* ShelfWidgets[4] = { ui.SlideShelf1, ui.SlideShelf2, ui.SlideShelf3, ui.SlideShelf4 };
	map<int, QPushButton*> mapLoadButtons = { {1, ui.btnReload1}, {2, ui.btnReload2}, {3, ui.btnReload3}, {4, ui.btnReload4 } };

	for (int j = 1; j <= 10; j++)
	{
		vecState[j - 1] = m_pControl->GetShelfHolderState(iIndex, j);
	}
	ShelfWidgets[iIndex - 1]->Refresh(vecState);
	if (!m_pControl->HoldsShelfAt(iIndex))
	{
		mapLoadButtons[iIndex]->setText("+");
	}
	else
	{
		mapLoadButtons[iIndex]->setText("-");
	}
}

void MainWindow::slotUpdateTransitShelfView()
{
	vector<int> vecState(10);

	for (int j = 1; j <= 10; j++)
	{
		vecState[j - 1] = m_pControl->TransitState(j);
	}
	ui.TransitShelf->Refresh(vecState);
}

void MainWindow::slotInterrupt(QString s)
{
	m_pControl->Suspend();
	ui.btnStart->setText(U8_TO_QSTR(u8"继续"));

	QMessageBox::information(this, U8_TO_QSTR(u8"中断"), s);
	this->slotMsg(s);
}

void MainWindow::slotMsg(QString strMsg)
{
	lock_guard<mutex> lk(m_mtxMsg);
	QString strLog = QDateTime::currentDateTime().toString("hh:mm:ss\t") + strMsg;
	ui.listWidgetMsg->addItem(strLog);
	LogMsg(strLog);
	if (ui.listWidgetMsg->count() >= 666)
	{
		ui.listWidgetMsg->takeItem(0);
	}

	ui.listWidgetMsg->setCurrentRow(ui.listWidgetMsg->count() - 1);
}

void MainWindow::slotQuestion(QString str)
{
	int iRet = QMessageBox::question(nullptr, U8_TO_QSTR(u8"question"), str);
	if (iRet == QMessageBox::Yes)
	{
		m_pControl->AnswerQuestion(true);
	}
	else if (iRet == QMessageBox::No)
	{
		m_pControl->AnswerQuestion(false);
	}
}

void MainWindow::slotRepairModAlarm(int iCode)
{
	if (m_pControl->IsRunning())
	{
		this->onBtnStartClicked();
	}
	else
	{
		m_pControl->Suspend();
	}

	mINI::INIStructure text;
	mINI::INIFile ini("./Config/Text.ini");
	ini.read(text);

	string strMsg = text[u8"修复模块报警"][to_string(iCode)];

	int iRet = QMessageBox::question(this, U8_TO_QSTR(u8"修复模块报警"), 
		U8_TO_QSTR(u8"报警原因【")
		.append(U8_TO_QSTR(strMsg))
		.append(U8_TO_QSTR(u8"】\r\n"))
		.append(U8_TO_QSTR(u8"继续加液（Yes/关闭），停止加液并运行（No）？")), 
		QMessageBox::StandardButton::Yes, QMessageBox::StandardButton::No);

	string strHandle;
	if (iRet != QMessageBox::No )
	{
		strHandle = text[u8"修复模块报警处理"][string(u8"继续加液").append(to_string(iCode))];
		m_pControl->HandleRepairModAlarm(strHandle.c_str());
	}
	else 
	{
		strHandle = text[u8"修复模块报警处理"][string(u8"继续运行").append(to_string(iCode))];
		m_pControl->HandleRepairModAlarm(strHandle.c_str());
	}
	this->onBtnStartClicked();
}

void MainWindow::slotRepairModAlarm2(int iAlarmIndex)
{
	if (m_pControl->IsRunning())
	{
		this->onBtnStartClicked();
	}
	else
	{
		m_pControl->Suspend();
	}
	//QMessageBox::information(this, U8_TO_QSTR(u8"修复模块报警"), U8_TO_QSTR(u8"报警原因【修复模块%1冷却排液时，液面传感器始终有信号。清洁传感器后继续。】").arg(iAlarmIndex - 6));
	QMessageBox::warning(this, U8_TO_QSTR(u8"修复模块报警"), U8_TO_QSTR(u8"报警原因【修复模块%1冷却排液时，液面传感器始终有信号。清洁传感器后继续。】").arg(iAlarmIndex - 6));
	m_pControl->HandleRepairModAlarm2(iAlarmIndex);

	this->onBtnStartClicked();
}

void MainWindow::slotUpdateReactModTemperateView(double dCDeg, int iIndex)
{
	(iIndex == 1 ? ui.lblReactModState1 : ui.lblReactModState2)->setText(QString::number(dCDeg) + U8_TO_QSTR(u8"°C"));
}

void MainWindow::slotUpdateRepairModTemperateView(double dCDeg, int iIndex)
{
	(iIndex == 1 ? ui.lblRepairModTemparature1 : ui.lblRepairModTemparature2)->setText(QString::number(dCDeg) + U8_TO_QSTR(u8"°C"));
}

void MainWindow::slotComboConfigTextChanged(const QString&)
{
	QMap<QString, QString> mapConfig = ui.comboConfigGroup->currentData().value<QMap<QString, QString>>();

	ui.tblConfigContents->clearContents();
	ui.tblConfigContents->setRowCount(mapConfig.count());
	int iCnt = 0;
	for (const QString& strKey : mapConfig.keys())
	{
		const QString& strValue = mapConfig[strKey];

		ui.tblConfigContents->setItem(iCnt, 0, new QTableWidgetItem(strKey));
		ui.tblConfigContents->setItem(iCnt, 1, new QTableWidgetItem(strValue));
		iCnt++;
	}
}

void MainWindow::slotComboCoordTypeTextChanged(const QString& strCoordType)
{
	const QMap<QString, QPair<int, QVector<double>>>& mapCoordData = ui.comboCoordType->currentData().value<QMap<QString, QPair<int, QVector<double>>>>();
	ui.comboCoordName->clear();
	for (const QString& strCoordName: mapCoordData.keys())
	{
		const QPair<int, QVector<double>>& coordData = mapCoordData[strCoordName];
		ui.comboCoordName->addItem(strCoordName, QVariant::fromValue<QPair<int, QVector<double>>>(coordData));
	}
}

void MainWindow::slotComboCoordNameTextChanged(const QString&)
{
	QPair<int, QVector<double>> coord = ui.comboCoordName->currentData().value<QPair<int, QVector<double>>>();
	int iMask = 1;
	for (int i = 0; i < 5; i++)
	{
		QLabel* lblCoord = dynamic_cast<QLabel*>(ui.hLayoutCoordValue->itemAt(i + 1)->widget());
		if (((iMask << i) & coord.first) != 0)
		{
			lblCoord->setText(QString::fromStdString(to_string(coord.second[i])));
		}
		else
		{
			lblCoord->setText("####");
		}
	}
}

void MainWindow::slotBtnMoveToCoordClicked()
{
	int iRet = m_pControl->MoveToCoord(QSTR_TO_U8(ui.comboCoordName->currentText()).c_str());
	if (iRet < 0)
	{
		/* error msg */
		QMessageBox::warning(this, "", U8_TO_QSTR(u8"移动失败"), QMessageBox::Ok);
	}
}

void MainWindow::onListAllReagentsCurrentItemChanged(QListWidgetItem* cur, QListWidgetItem* prev)
{
	const int iCnt = ui.listAllReagents->count();
	const int iCur = ui.listAllReagents->currentRow();
	if (iCnt == 0 || iCur == -1)
	{
		ui.btnRemoveReagent->setDisabled(true);
		ui.btnMoveUpReagent->setDisabled(true);
		ui.btnMoveDownReagent->setDisabled(true);
	}
	else
	{
		ui.btnRemoveReagent->setEnabled(true);
		ui.btnMoveDownReagent->setEnabled(true);
		ui.btnMoveUpReagent->setEnabled(true);
		if (iCur == 0)
		{
			ui.btnMoveUpReagent->setDisabled(true);
		}
		if (iCur == iCnt - 1)
		{
			ui.btnMoveDownReagent->setDisabled(true);
		}
	}
}

void MainWindow::onListFormulaDetailCurrentItemChanged(QListWidgetItem* cur, QListWidgetItem* prev)
{
	const int iCnt = ui.listFormulaDetail->count();
	const int iCur = ui.listFormulaDetail->currentRow();
	if (iCnt == 0 || iCur == -1)
	{
		ui.btnRemoveFormulaRow->setDisabled(true);
		ui.btnMoveupFormulaRow->setDisabled(true);
		ui.btnMovedownFormulaRow->setDisabled(true);
	}
	else
	{
		ui.btnRemoveFormulaRow->setEnabled(true);
		ui.btnMoveupFormulaRow->setEnabled(true);
		ui.btnMovedownFormulaRow->setEnabled(true);
		if (iCur == 0)
		{
			ui.btnMoveupFormulaRow->setDisabled(true);
		}
		if (iCur == iCnt - 1)
		{
			ui.btnMovedownFormulaRow->setDisabled(true);
		}
	}
}

void MainWindow::onBtnAddReagentClicked()
{
	int iCurRow = ui.listAllReagents->currentRow();
	ui.listAllReagents->insertItem(iCurRow + 1, "####");
	ui.listAllReagents->item(iCurRow + 1)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable);
	ui.listAllReagents->item(iCurRow + 1)->setSizeHint(QSize(ui.listAllReagents->width() - 10, 40));
	ui.listAllReagents->setCurrentRow(iCurRow + 1);
	onListAllReagentsCurrentItemChanged(nullptr, nullptr);
}

void MainWindow::onBtnRemoveReagentClicked()
{
	ui.listAllReagents->takeItem(ui.listAllReagents->currentRow());
	onListAllReagentsCurrentItemChanged(nullptr, nullptr);
}

void MainWindow::onBtnMoveUpReagentClicked()
{
	const int iCur = ui.listAllReagents->currentRow();
	if (iCur > 0)
	{
		QListWidgetItem* pCurItem = ui.listAllReagents->takeItem(iCur);
		ui.listAllReagents->insertItem(iCur - 1, pCurItem);
		ui.listAllReagents->setCurrentRow(iCur - 1);
		onListAllReagentsCurrentItemChanged(nullptr, nullptr);
	}
}

void MainWindow::onBtnMoveDownReagentClicked()
{
	const int iCur = ui.listAllReagents->currentRow();
	const int iPrevCount = ui.listAllReagents->count();
	if (iCur < iPrevCount - 1)
	{
		QListWidgetItem* pCurItem = ui.listAllReagents->takeItem(iCur);
		ui.listAllReagents->insertItem(iCur + 1, pCurItem);
		ui.listAllReagents->setCurrentRow(iCur + 1);
		onListAllReagentsCurrentItemChanged(nullptr, nullptr);
	}
}

void MainWindow::onBtnSaveReagentClicked()
{
	mINI::INIFile ini("./Config/Reagent.ini");
	mINI::INIStructure reagents;
	ini.write(reagents);

	int iCnt = ui.listAllReagents->count();
	for (int i = 0; i < iCnt; i++)
	{
		reagents["Reagent"][QSTR_TO_U8(ui.listAllReagents->item(i)->text())] = "0";
	}

	ini.write(reagents);

	int iOldCnt = ui.listAllReagents->count();
	int iOldSel = ui.listAllReagents->currentRow();
	//ReloadReagentView();
	if (ui.listAllReagents->count() == iOldCnt)
	{
		ui.listAllReagents->setCurrentRow(iOldSel);
	}
}

void MainWindow::onComboReagentShelfChanged(const QString&)
{
	ui.listReagentShelfDetails->clear();
	const QMap<int, QString>& mapComboData = ui.comboReagentShelves->itemData(ui.comboReagentShelves->currentIndex()).value<QMap<int, QString>>();
	QStringList strlistAllReagents;
	for (int i = 0; i < ui.listAllReagents->count(); i++)
	{
		strlistAllReagents.append(ui.listAllReagents->item(i)->text());
	}

	for (int i = 1; i <= 18; i++)
	{
		QListWidgetItem* pItem = new QListWidgetItem();
		pItem->setSizeHint(QSize(ui.listReagentShelfDetails->width() - 10, 40));
		ui.listReagentShelfDetails->addItem(pItem);
		if (mapComboData.contains(i))
		{
			ui.listReagentShelfDetails->setItemWidget(pItem, new ReagentShelfRow(ui.listReagentShelfDetails, i, true, mapComboData[i], strlistAllReagents));
		}
		else
		{
			ui.listReagentShelfDetails->setItemWidget(pItem, new ReagentShelfRow(ui.listReagentShelfDetails, i, false, "", strlistAllReagents));
		}
	}
}

void MainWindow::onBtnSaveReagentShelfClicked()
{
	mINI::INIFile ini("./Config/ReagentShelf.ini");
	mINI::INIStructure rs;
	ini.read(rs);

	string strRSName = QSTR_TO_U8(ui.comboReagentShelves->currentText());
	rs[strRSName].clear();
	for (int i = 1; i <= 18; i++)
	{
		ReagentShelfRow* pReagent = dynamic_cast<ReagentShelfRow*>(ui.listReagentShelfDetails->itemWidget(ui.listReagentShelfDetails->item(i - 1)));
		if (pReagent->NonEmpty())
		{
			rs[strRSName][to_string(i)] = pReagent->GetReagentName();
		}
	}

	ini.write(rs);
	ReloadReagentShelfView();
	ui.comboReagentShelves->setCurrentText(U8_TO_QSTR(strRSName));

	m_pControl->ReloadReagentShelf();

	QMessageBox::information(this, "", U8_TO_QSTR(u8"已保存"));
}

void MainWindow::onComboFormulaChanged(int)
{
	ui.listFormulaDetail->clear();
	ui.btnAddFormulaRow->setEnabled(true);
	ui.btnRemoveFormula->setEnabled(true);
	ui.btnSaveFormula->setEnabled(true);
	if (ui.comboBoxFormula->count() <= 0)
	{
		ui.btnRemoveFormula->setDisabled(true);
		ui.btnSaveFormula->setDisabled(true);
		ui.btnAddFormulaRow->setDisabled(true);
		ui.btnRemoveFormulaRow->setDisabled(true);
		ui.btnMoveupFormulaRow->setDisabled(true);
		ui.btnMovedownFormulaRow->setDisabled(true);
		return;
	}

	string strCurrentFormula = QSTR_TO_U8(ui.comboBoxFormula->currentText());
	string strFilename = StrToHex(strCurrentFormula);
	mINI::INIFile ini(string("./Formula/").append( strFilename));
	mINI::INIStructure formula;
	ini.read(formula);

	QStringList strlistAllReagent = GetAllReagent();

	int iIndex = 1;
	for (auto row : formula[strCurrentFormula])
	{
		QStringList strlistRowContent = QString::fromUtf8(row.second).split(",");
		
		QString strType = strlistRowContent.at(0);
		QString strReagent = strlistRowContent.at(1);
		int iQuantity = strlistRowContent.at(2).toInt();
		int iTime = strlistRowContent.at(3).toInt();

		QListWidgetItem* pItem = new QListWidgetItem();
		pItem->setSizeHint(QSize(0, 40));
		ui.listFormulaDetail->addItem(pItem);
		ui.listFormulaDetail->setItemWidget(pItem, new FormulaRow(nullptr, iIndex, strType, strlistAllReagent, strReagent, iQuantity, iTime));
		iIndex++;
	}
}

void MainWindow::onBtnAddFormulaRowClicked()
{
	int iCurRow = ui.listFormulaDetail->currentRow();
	QListWidgetItem* pItem = new QListWidgetItem();
	pItem->setSizeHint(QSize(0, 40));
	ui.listFormulaDetail->insertItem(iCurRow + 1, pItem);
	ui.listFormulaDetail->setItemWidget(pItem, new FormulaRow(nullptr, iCurRow + 1 + 1, TYPE_CREAGENT, GetAllReagent(), "", 50, 120));
	for (int i = iCurRow + 2; i < ui.listFormulaDetail->count(); i++)
	{
		dynamic_cast<FormulaRow*>(ui.listFormulaDetail->itemWidget(ui.listFormulaDetail->item(i)))->IncIndex();
	}
	ui.listFormulaDetail->setCurrentRow(iCurRow + 1);
	onListFormulaDetailCurrentItemChanged(nullptr, nullptr);
}

void MainWindow::onBtnRemoveFormulaRowClicked()
{
	int iCurRow = ui.listFormulaDetail->currentRow();
	delete(ui.listFormulaDetail->takeItem(iCurRow));
	for (int i = iCurRow; i < ui.listFormulaDetail->count(); i++)
	{
		dynamic_cast<FormulaRow*>(ui.listFormulaDetail->itemWidget(ui.listFormulaDetail->item(i)))->DecIndex();
	}
	onListFormulaDetailCurrentItemChanged(nullptr, nullptr);
}

void MainWindow::onBtnMoveUpFormulaRowClicked()
{
	const int iCurRow = ui.listFormulaDetail->currentRow();
	if (iCurRow < 1)
		return;

	FormulaRow* pRow = new FormulaRow(GetAllReagent(), * dynamic_cast<FormulaRow*>(ui.listFormulaDetail->itemWidget(ui.listFormulaDetail->item(iCurRow))));
	pRow->DecIndex();
	QListWidgetItem* pItem = ui.listFormulaDetail->takeItem(iCurRow);
	ui.listFormulaDetail->insertItem(iCurRow - 1, pItem);
	ui.listFormulaDetail->setCurrentRow(iCurRow - 1);
	ui.listFormulaDetail->setItemWidget(pItem, pRow);

	dynamic_cast<FormulaRow*>(ui.listFormulaDetail->itemWidget(ui.listFormulaDetail->item(iCurRow)))->IncIndex();

	onListFormulaDetailCurrentItemChanged(nullptr, nullptr);
}

void MainWindow::onBtnMoveDownFormulaRowClicked()
{
	const int iCurRow = ui.listFormulaDetail->currentRow();
	if (iCurRow >= ui.listFormulaDetail->count() - 1)
		return;

	FormulaRow* pRow = new FormulaRow(GetAllReagent(), *dynamic_cast<FormulaRow*>(ui.listFormulaDetail->itemWidget(ui.listFormulaDetail->item(iCurRow))));
	pRow->IncIndex();
	QListWidgetItem* pItem = ui.listFormulaDetail->takeItem(iCurRow);
	ui.listFormulaDetail->insertItem(iCurRow + 1, pItem);
	ui.listFormulaDetail->setCurrentRow(iCurRow + 1);
	ui.listFormulaDetail->setItemWidget(pItem, pRow);

	dynamic_cast<FormulaRow*>(ui.listFormulaDetail->itemWidget(ui.listFormulaDetail->item(iCurRow)))->DecIndex();

	onListFormulaDetailCurrentItemChanged(nullptr, nullptr);
}

void MainWindow::onBtnSaveFormulaClicked()
{
	for (int i = 0; i < ui.listFormulaDetail->count(); i++)
	{
		if (!dynamic_cast<FormulaRow*>(ui.listFormulaDetail->itemWidget(ui.listFormulaDetail->item(i)))->CReagentAssigned())
		{
			QMessageBox::information(nullptr, QString::fromUtf8(u8""), QString::fromUtf8(u8"试剂未指定"));
			return;
		}
	}



	int iRet = QMessageBox::question(nullptr, "", U8_TO_QSTR(u8"保存？"));
	if (iRet == QMessageBox::Yes)
	{
		string strCurrentFormula = QSTR_TO_U8(ui.comboBoxFormula->currentText());
		string strFilename = StrToHex(strCurrentFormula);
		mINI::INIFile ini(string("./Formula/").append(strFilename));
		mINI::INIStructure formula;

		for (int i = 0; i < ui.listFormulaDetail->count(); i++)
		{
			FormulaRow* pRow = dynamic_cast<FormulaRow*>(ui.listFormulaDetail->itemWidget(ui.listFormulaDetail->item(i)));
			string strIniVal = pRow->GetRowText().toStdString();
			formula[strCurrentFormula][to_string(i + 1)] = strIniVal;
		}

		ini.write(formula);

		QMessageBox::information(nullptr, "", U8_TO_QSTR(u8"重启软件后生效"));
	}
}

void MainWindow::onBtnCreateFormulaClicked()
{
	QString strInput = QInputDialog::getText(nullptr, U8_TO_QSTR(u8"新建程序"), U8_TO_QSTR(u8"程序名称"));
	if (!strInput.isEmpty())
	{
		if (GetAllFormula().contains(strInput))
		{
			QMessageBox::warning(nullptr, U8_TO_QSTR("失败"), U8_TO_QSTR("已存在的程序名称"));
		}
		else
		{
			mINI::INIFile ini("./Formula/All");
			mINI::INIStructure allFml;
			ini.read(allFml);
			allFml["AllFormula"][QSTR_TO_U8(strInput)] = "0";
			ini.write(allFml);

			mINI::INIFile iniFormulaFile(string("./Formula/").append(StrToHex(QSTR_TO_U8(strInput))));
			mINI::INIStructure iniFormula;
			iniFormula[QSTR_TO_U8(strInput)]; /* create empty section */
			iniFormulaFile.write(iniFormula);
			this->ReloadFormulaView();

			ui.comboBoxFormula->setCurrentText(strInput);
		}
	}
}

void MainWindow::onBtnDeleteFormulaClicked()
{
	int iRet = QMessageBox::question(nullptr, U8_TO_QSTR(u8"删除程序"), U8_TO_QSTR(u8"确认删除程序？不可恢复。"));
	if (iRet != QMessageBox::Yes)
		return;

	string strCurrentFormula = QSTR_TO_U8(ui.comboBoxFormula->currentText());
	mINI::INIFile ini("./Formula/All");
	mINI::INIStructure allFml;
	ini.read(allFml);
	allFml["AllFormula"].remove(strCurrentFormula);
	ini.write(allFml);

	ui.comboBoxFormula->removeItem(ui.comboBoxFormula->findText(U8_TO_QSTR(strCurrentFormula), Qt::MatchExactly));
	
	iRet = ::remove(string("./Formula/").append(StrToHex(strCurrentFormula)).c_str());
}

void MainWindow::ReloadConfigView()
{    
	/* read from file */
	mINI::INIFile fileCfg("./Config/Config.ini");
	mINI::INIStructure configContent;
	fileCfg.read(configContent);
	
	/* set `config` combobox */
	for (auto config : configContent)
	{
		const string& strSection = config.first;
		QMap<QString, QString> mapConfigData;
		for (auto keyVal : config.second)
		{
			mapConfigData[U8_TO_QSTR(keyVal.first)] = U8_TO_QSTR(keyVal.second);
		}
		ui.comboConfigGroup->addItem(U8_TO_QSTR(strSection), QVariant::fromValue<QMap<QString, QString>>(mapConfigData));
	}

	/* IO - PLC combobox */
	mINI::INIStructure ioCfg;
	mINI::INIFile("./Config/IO.ini").read(ioCfg);
	QStringList strIOPLCStationList = { U8_TO_QSTR(u8"修复模块"), U8_TO_QSTR(u8"反应模块1"), U8_TO_QSTR(u8"反应模块2") };
	
	QMap<QString, QMap<QString, QString>> mapComboDataRepairMod;
	QMap<QString, QMap<QString, QString>> mapComboDataReactMod;
	for (auto& section : ioCfg)
	{
		const string& strSectionName = section.first;
		QMap<QString, QString> mapIniSection;
		for (auto& cfg : section.second)
		{
			mapIniSection[U8_TO_QSTR(cfg.first)] = U8_TO_QSTR(cfg.second);
		}

		if (strSectionName.find(u8"修复模块") != -1)
		{
			mapComboDataRepairMod[U8_TO_QSTR(strSectionName)] = mapIniSection;
		}
		else if (strSectionName.find(u8"反应模块") != -1)
		{
			mapComboDataReactMod[U8_TO_QSTR(strSectionName)] = mapIniSection;
		}
	}

	for (const QString& strDevName : strIOPLCStationList)
	{
		int iStationID = stoi(ioCfg["Station"][QSTR_TO_U8(strDevName)]);
		if(strDevName.contains(U8_TO_QSTR(u8"修复模块")))
			ui.comboBoxPLCStation->addItem(strDevName, QVariant::fromValue(QPair<int, QMap<QString, QMap<QString, QString>>>(iStationID, mapComboDataRepairMod)));
		else if(strDevName.contains(U8_TO_QSTR(u8"反应模块")))
			ui.comboBoxPLCStation->addItem(strDevName, QVariant::fromValue(QPair<int, QMap<QString, QMap<QString, QString>>>(iStationID, mapComboDataReactMod)));
	}	
}

void MainWindow::ReloadCoordView()
{
	mINI::INIFile fileCoord("./Config/Position.pt");
	mINI::INIStructure coordContent;
	fileCoord.read(coordContent);

	/* set `coord` combobox */
	QMap<QString, QMap<QString, QPair<int, QVector<double>>>> mapCoordData;
	for (auto coord : coordContent)
	{
		const string& strSection = coord.first;
		int iComboIndex = -1;
		for (int i = 0; i < ui.comboCoordType->count(); i++) /* this combobox is set in .ui file */
		{
			string strCoordTypeName = QSTR_TO_U8(ui.comboCoordType->itemText(i));
			if (strSection.find(strCoordTypeName) != strSection.npos)
			{
				iComboIndex = i;
				//mapCoordData[strCoordTypeName][U8_TO_QSTR(strSection)];
				break;
			}
		}

		int iAxisBitmap = stoi(coord.second["AxisNos"]);
		QVector<double> vecCoords(5);
		int iMask = 1;
		for (int j = 0; j < 5; j++)
		{
			if ((iAxisBitmap & (iMask << j)) != 0)
			{
				vecCoords[j] = stod(coord.second[to_string(j)]);
			}
		}

		const QString& strCoordTypeName = (iComboIndex < 0) ? u8"其他" : ui.comboCoordType->itemText(iComboIndex);
		mapCoordData[strCoordTypeName][U8_TO_QSTR(strSection)] = QPair<int, QVector<double>>(iAxisBitmap, vecCoords);
	}

	ui.comboCoordType->clear();
	for (auto coordType : mapCoordData.keys())
	{
		ui.comboCoordType->addItem(coordType, QVariant::fromValue<QMap<QString, QPair<int, QVector<double>>>>(mapCoordData[coordType]));
	}
}

void MainWindow::ReloadReagentView()
{
	ui.listAllReagents->clear();
	mINI::INIFile ini("./Config/Reagent.ini");
	mINI::INIStructure reagents;
	ini.read(reagents);

	int iRow = 0;
	for (auto iter : reagents["Reagent"])
	{
		ui.listAllReagents->addItem(U8_TO_QSTR(iter.first));
		ui.listAllReagents->item(iRow)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable);
		ui.listAllReagents->item(iRow)->setSizeHint(QSize(0, 40));
		iRow++;
	}

	onListAllReagentsCurrentItemChanged(nullptr, nullptr);
}

void MainWindow::ReloadFormulaView()
{
	ui.comboBoxFormula->clear();

	mINI::INIFile ini("./Formula/All");
	mINI::INIStructure allFml;
	ini.read(allFml);

	for (auto fml : allFml["AllFormula"])
	{
		const string& strFormulaName = fml.first;
		ui.comboBoxFormula->addItem(U8_TO_QSTR(strFormulaName));
	}
	if (ui.comboBoxFormula->count() <= 0)
	{
		ui.btnAddFormulaRow->setDisabled(true);
	}

	onListFormulaDetailCurrentItemChanged(nullptr, nullptr);
}

void MainWindow::ReloadReagentShelfView()
{
	mINI::INIFile ini("./Config/ReagentShelf.ini");
	mINI::INIStructure rs;
	ini.read(rs);

	ui.comboReagentShelves->clear();

	for (auto iter : rs)
	{
		const string& strReagentShelfName = iter.first;
		QMap<int, QString> listComboData;
		for (auto iterReagent : iter.second)
		{
			const string& strPos = iterReagent.first; /* 1~18 */
			const string& strReagentName = iterReagent.second;
			listComboData[stoi(strPos)] = U8_TO_QSTR(strReagentName);
		}
		ui.comboReagentShelves->addItem(U8_TO_QSTR(strReagentShelfName), QVariant::fromValue<QMap<int, QString>>(listComboData));
	}
}

void MainWindow::MachineReloadShelf(int iIndex)
{
	map<int, QPushButton*> mapLoadButtons = { {1, ui.btnReload1}, {2, ui.btnReload2}, {3, ui.btnReload3}, {4, ui.btnReload4 } };

	const string strFilename = string(u8"./Product/SS").append(to_string(iIndex)).append(".pro");
	bool bHasShelf = m_pControl->HoldsShelfAt(iIndex);
	if (bHasShelf)
	{
		const QString& strMsg = U8_TO_QSTR(u8"取走玻片架？");
		int iRet = QMessageBox::question(nullptr, "", strMsg, QMessageBox::StandardButton::Ok, QMessageBox::StandardButton::Cancel);

		if (iRet == QMessageBox::Ok)
		{
			m_pControl->ClearShelfHolder(iIndex);
			mINI::INIStructure empty;
			mINI::INIFile(strFilename).write(empty);
			mapLoadButtons[iIndex]->setText("+");
		}
	}
	else
	{
		SlideShelfEditDialog dlg(this->GetAllFormula(), this->GetAllReagent());
		int iRet = dlg.exec();
		if (iRet == QDialog::Accepted)
		{
			dlg.Save(U8_TO_QSTR(strFilename));
			m_pControl->ReloadShelfHolder(iIndex);
			if (m_pControl->HoldsShelfAt(iIndex))
			{
				mapLoadButtons[iIndex]->setText("-");
			}
		}
	}

	this->slotUpdateShelfHolderView(iIndex);
}

