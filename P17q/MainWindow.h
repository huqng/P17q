#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_MainWindow.h"

#include "../MachineControlQt/IMachineControl.h"

class QCloseEvent;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = Q_NULLPTR);
    ~MainWindow();
protected:
    virtual void closeEvent(QCloseEvent* evnt)override;

private:
    Ui::MainWindowClass ui;

    CIMachineControl* m_pControl; 
public slots:
    /* home */
    void onBtnResetClicked();
    void onBtnStartClicked();
    void onBtnStopClicked();
    void onBtnTestClicked();
    void onBtnReload1Clicked();
    void onBtnReload2Clicked();
    void onBtnReload3Clicked();
    void onBtnReload4Clicked();
    void onBtnClearMsgClicked();

    /* machine control */
    void slotResetFinished();
    void slotUpdateHomeView();    
    void slotUpdateReactModView(int iIndex);
    void slotUpdateRepairModView(int iIndex);
    void slotUpdateShelfHolderView(int iIndex);
    void slotUpdateTransitShelfView();
    void slotInterrupt(QString);
    void slotMsg(QString str);
    void slotQuestion(QString str);
    void slotRepairModAlarm(int);
    void slotRepairModAlarm2(int);
    void slotUpdateReactModTemperateView(double dCDeg, int iIndex);
    void slotUpdateRepairModTemperateView(double dCDeg, int iIndex);
    
    /* param */
    void slotComboConfigTextChanged(const QString&);
    void slotComboCoordTypeTextChanged(const QString&);
    void slotComboCoordNameTextChanged(const QString&);
    void slotBtnMoveToCoordClicked();
    void onBtnSaveConfigClicked();
    void onBtnSaveCoordClicked();
    void onBtnCoordMoveNegtiveX();
    void onBtnCoordMovePositiveX();
    void onBtnCoordMoveNegtiveY();
    void onBtnCoordMovePositiveY();
    void onBtnCoordMoveNegtiveZ1();
    void onBtnCoordMovePositiveZ1();
    void onBtnCoordMoveNegtiveZ2();
    void onBtnCoordMovePositiveZ2();
    void onBtnCoordMoveNegtiveZ3();
    void onBtnCoordMovePositiveZ3();
    void onBtnSingleAxisHome1Clicked();
    void onBtnSingleAxisHome2Clicked();
    void onBtnSingleAxisHome3Clicked();
    void onBtnSingleAxisHome4Clicked();
    void onBtnSingleAxisHome5Clicked();
    void onBtnEmgStopClicked();
    void onBtnZAxisUpClicked();
    void slotUpdateCoord(double, double, double, double, double);
    void onCheckBoxIO0Clicked(bool bChecked);
    void onCheckBoxIO1Clicked(bool bChecked);
    void onCheckBoxIO2Clicked(bool bChecked);
    void onBtnCalcReagentPosClicked();
    void onBtnCalcSlidePos1Clicked();
    void onBtnCalcSlidePos2Clicked();
    /* plc mod */
    void onBtnWriteDTClicked();
    void onBtnReadDTClicked();
    void onBtnSetRelayClicked();
    void onBtnClearRelayClicked();
    void onBtnReadRelayClicked();
    void onComboPLCStationTextChanged(const QString&);

    /* reagents */
    void onListAllReagentsCurrentItemChanged(QListWidgetItem* cur, QListWidgetItem* prev);
    void onListFormulaDetailCurrentItemChanged(QListWidgetItem* cur, QListWidgetItem* prev);
    void onBtnAddReagentClicked();
    void onBtnRemoveReagentClicked();
    void onBtnMoveUpReagentClicked();
    void onBtnMoveDownReagentClicked();
    void onBtnSaveReagentClicked();
    /* reagent shelf editor */
    void onComboReagentShelfChanged(const QString&);
    void onBtnSaveReagentShelfClicked();
    /* formula editor */
    void onComboFormulaChanged(int);
    void onBtnAddFormulaRowClicked();
    void onBtnRemoveFormulaRowClicked();
    void onBtnMoveUpFormulaRowClicked();
    void onBtnMoveDownFormulaRowClicked();
    void onBtnSaveFormulaClicked();
    void onBtnCreateFormulaClicked();
    void onBtnDeleteFormulaClicked();


signals:

private:
    void MachineReloadShelf(int iIndex);
    void ReloadConfigView();
    void ReloadCoordView();
    void ReloadReagentView();
    void ReloadFormulaView();
    void ReloadReagentShelfView();

    double GetMoveLength();
    void ImplCalcSlidePos(int iReactModIndex);

    QStringList GetAllReagent();
    QStringList GetAllFormula();

    mutex m_mtxMsg;
    void LogMsg(QString str);
};

string StrToHex(string strIn);
