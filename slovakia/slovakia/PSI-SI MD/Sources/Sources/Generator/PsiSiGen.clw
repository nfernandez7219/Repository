; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CMainFrame
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "psisigen.h"
LastPage=0

ClassCount=6
Class1=Splitter
Class2=CMainFrame
Class3=CPsiSiGenApp
Class4=CAboutDlg

ResourceCount=4
Resource1=IDD_PhysInterface
Resource2=IDD_TablesFreq
Class5=CTablesFreqDlg
Resource3=IDD_ABOUTBOX
Class6=CPhysInterfaceDlg
Resource4=IDR_MAINFRAME

[CLS:Splitter]
Type=0
BaseClass=CMDIChildWnd
HeaderFile=Splitter.h
ImplementationFile=Splitter.cpp

[CLS:CMainFrame]
Type=0
BaseClass=CFrameWnd
HeaderFile=Src\MainFrm.h
ImplementationFile=Src\MainFrm.cpp
Filter=T
VirtualFilter=fWC
LastObject=ID_FILE_OPEN_DontApply

[CLS:CPsiSiGenApp]
Type=0
BaseClass=CWinApp
HeaderFile=Src\PsiSiGen.h
ImplementationFile=Src\PsiSiGen.cpp
LastObject=CPsiSiGenApp

[CLS:CAboutDlg]
Type=0
BaseClass=CDialog
HeaderFile=Src\PsiSiGen.cpp
ImplementationFile=Src\PsiSiGen.cpp
LastObject=CAboutDlg
Filter=D
VirtualFilter=dWC

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=5
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342312961
Control3=IDC_STATIC,static,1342312576
Control4=IDOK,button,1342242817
Control5=IDC_ChangePrgLevel,button,1342242816

[TB:IDR_MAINFRAME]
Type=1
Class=?
Command1=IDC_Start
Command2=IDC_Stop
Command3=ID_OUTPUT_TABLESFREQUENCIES
Command4=ID_FILE_OPEN
Command5=ID_FILE_SAVE
Command6=ID_APP_ABOUT
CommandCount=6

[MNU:IDR_MAINFRAME]
Type=1
Class=?
Command1=IDC_Start
Command2=IDC_Stop
Command3=ID_APP_EXIT
Command4=ID_VIEWEDIT_VIEWRUNNINGPSITABLES
Command5=ID_VIEWEDIT_VIEWEDITEDPSITABLES
Command6=ID_VIEWEDIT_EDITPSITABLES
Command7=ID_OUTPUT_TABLESFREQUENCIES
Command8=ID_Device_DvbAsi
Command9=ID_Device_Dvb
Command10=ID_Device_Ethernet
Command11=ID_OUTPUT_PHYSICALINTERFACE
Command12=ID_NewNetworkWizard
Command13=ID_AddProgramWizard
Command14=ID_FILE_OPEN
Command15=ID_FILE_OPEN_DontApply
Command16=ID_FILE_SAVE
Command17=ID_FILE_SAVE_AS
Command18=ID_APP_ABOUT
CommandCount=18

[ACL:IDR_MAINFRAME]
Type=1
Class=?
Command1=ID_EDIT_COPY
Command2=ID_EDIT_PASTE
Command3=ID_EDIT_UNDO
Command4=ID_EDIT_CUT
Command5=ID_NEXT_PANE
Command6=ID_PREV_PANE
Command7=ID_EDIT_COPY
Command8=ID_EDIT_PASTE
Command9=ID_EDIT_CUT
Command10=ID_EDIT_UNDO
Command11=ID_CONTEXT_HELP
Command12=ID_HELP
CommandCount=12

[DLG:IDR_MAINFRAME]
Type=1
Class=?
ControlCount=0

[DLG:IDD_TablesFreq]
Type=1
Class=CTablesFreqDlg
ControlCount=40
Control1=IDC_STATIC,button,1342177287
Control2=IDC_chkPAT,button,1342242819
Control3=IDC_STATIC,static,1342308352
Control4=IDC_PatFreq,edit,1350639744
Control5=IDC_STATIC,static,1342308352
Control6=IDC_PatSpeed,edit,1350568064
Control7=IDC_STATIC,static,1342308352
Control8=IDC_STATIC,button,1342177287
Control9=IDC_chkPMT,button,1342242819
Control10=IDC_STATIC,static,1342308352
Control11=IDC_PmtFreq,edit,1350639744
Control12=IDC_STATIC,static,1342308352
Control13=IDC_PmtSpeed,edit,1350568064
Control14=IDC_STATIC,static,1342308352
Control15=IDC_STATIC,button,1342177287
Control16=IDC_chkSDT,button,1342242819
Control17=IDC_STATIC,static,1342308352
Control18=IDC_SdtFreq,edit,1350639744
Control19=IDC_STATIC,static,1342308352
Control20=IDC_SdtSpeed,edit,1350568064
Control21=IDC_STATIC,static,1342308352
Control22=IDC_STATIC,button,1342177287
Control23=IDC_chkNIT,button,1342242819
Control24=IDC_STATIC,static,1342308352
Control25=IDC_NitFreq,edit,1350639744
Control26=IDC_STATIC,static,1342308352
Control27=IDC_NitSpeed,edit,1350568064
Control28=IDC_STATIC,static,1342308352
Control29=IDC_STATIC,button,1342177287
Control30=IDC_chkCAT,button,1342242819
Control31=IDC_STATIC,static,1342308352
Control32=IDC_CatFreq,edit,1350639744
Control33=IDC_STATIC,static,1342308352
Control34=IDC_CatSpeed,edit,1350568064
Control35=IDC_STATIC,static,1342308352
Control36=IDC_STATIC,static,1342308352
Control37=IDC_TotalSpeed,edit,1350568064
Control38=IDC_STATIC,static,1342308352
Control39=IDOK,button,1342242817
Control40=IDCANCEL,button,1342242816

[CLS:CTablesFreqDlg]
Type=0
HeaderFile=src\tablesfreqdlg.h
ImplementationFile=src\tablesfreqdlg.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=CTablesFreqDlg

[DLG:IDD_PhysInterface]
Type=1
Class=CPhysInterfaceDlg
ControlCount=25
Control1=IDC_btnDvbAsi,button,1342308361
Control2=IDC_btnRs422,button,1342177289
Control3=IDC_btnEthernet,button,1342177289
Control4=IDC_STATIC,static,1342308352
Control5=IDC_edtStuffing,edit,1350639744
Control6=IDC_chkNoIbStuffing,button,1342242819
Control7=IDC_STATIC,static,1342308352
Control8=IDC_edtComergonPort,edit,1350639616
Control9=IDC_STATIC,static,1342308352
Control10=IDC_edtComergonIrq,edit,1350639616
Control11=IDC_edtComergonTest,button,1342242816
Control12=IDC_STATIC,static,1342308352
Control13=IDC_edtIpAddress,SysIPAddress32,1342242816
Control14=IDC_STATIC,static,1342308352
Control15=IDC_edtIpPort,edit,1350639744
Control16=IDC_STATIC,button,1342177287
Control17=IDC_btnUdp,button,1342308361
Control18=IDC_btnTcp,button,1342177289
Control19=IDOK,button,1342242817
Control20=IDCANCEL,button,1342242816
Control21=IDC_STATIC,button,1342177287
Control22=IDC_STATIC,button,1342177287
Control23=IDC_STATIC,button,1342177287
Control24=IDC_STATIC,static,1342308352
Control25=IDC_STATIC,static,1342308352

[CLS:CPhysInterfaceDlg]
Type=0
HeaderFile=src\physinterfacedlg.h
ImplementationFile=src\physinterfacedlg.cpp
BaseClass=CDialog
Filter=D
LastObject=IDC_btnTcp
VirtualFilter=dWC

