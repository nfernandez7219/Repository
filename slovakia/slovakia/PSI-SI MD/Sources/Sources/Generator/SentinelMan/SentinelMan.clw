; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CSentinelManDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "SentinelMan.h"

ClassCount=4
Class1=CSentinelManApp
Class2=CSentinelManDlg
Class3=CAboutDlg

ResourceCount=3
Resource1=IDD_SENTINELMAN_DIALOG
Resource2=IDR_MAINFRAME
Class4=OperNameDlg
Resource3=IDD_OperatorName

[CLS:CSentinelManApp]
Type=0
HeaderFile=SentinelMan.h
ImplementationFile=SentinelMan.cpp
Filter=N

[CLS:CSentinelManDlg]
Type=0
HeaderFile=SentinelManDlg.h
ImplementationFile=SentinelManDlg.cpp
Filter=D
BaseClass=CDialog
VirtualFilter=dWC
LastObject=IDC_cbApplication

[CLS:CAboutDlg]
Type=0
HeaderFile=SentinelManDlg.h
ImplementationFile=SentinelManDlg.cpp
Filter=D

[DLG:IDD_SENTINELMAN_DIALOG]
Type=1
Class=CSentinelManDlg
ControlCount=6
Control1=IDC_STATIC,static,1342308352
Control2=IDC_cbApplication,combobox,1344340035
Control3=IDC_STATIC,static,1342308352
Control4=IDC_lstOperators,listbox,1353777411
Control5=IDC_NewOperator,button,1342242816
Control6=IDC_DelOperator,button,1342242816

[DLG:IDD_OperatorName]
Type=1
Class=OperNameDlg
ControlCount=4
Control1=IDC_STATIC,static,1342308352
Control2=IDC_edtOperName,edit,1350631552
Control3=IDOK,button,1342242817
Control4=IDCANCEL,button,1342242816

