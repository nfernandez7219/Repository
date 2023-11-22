@echo off
REM -- First make map file from Microsoft Visual C++ generated resource.h
echo // MAKEHELP.BAT generated Help Map file.  Used by PSISIGEN.HPJ. >"hlp\PsiSiGen.hm"
echo. >>"hlp\PsiSiGen.hm"
echo // Commands (ID_* and IDM_*) >>"hlp\PsiSiGen.hm"
makehm ID_,HID_,0x10000 IDM_,HIDM_,0x10000 resource.h >>"hlp\PsiSiGen.hm"
echo. >>"hlp\PsiSiGen.hm"
echo // Prompts (IDP_*) >>"hlp\PsiSiGen.hm"
makehm IDP_,HIDP_,0x30000 resource.h >>"hlp\PsiSiGen.hm"
echo. >>"hlp\PsiSiGen.hm"
echo // Resources (IDR_*) >>"hlp\PsiSiGen.hm"
makehm IDR_,HIDR_,0x20000 resource.h >>"hlp\PsiSiGen.hm"
echo. >>"hlp\PsiSiGen.hm"
echo // Dialogs (IDD_*) >>"hlp\PsiSiGen.hm"
makehm IDD_,HIDD_,0x20000 resource.h >>"hlp\PsiSiGen.hm"
echo. >>"hlp\PsiSiGen.hm"
echo // Frame Controls (IDW_*) >>"hlp\PsiSiGen.hm"
makehm IDW_,HIDW_,0x50000 resource.h >>"hlp\PsiSiGen.hm"
REM -- Make help for Project PSISIGEN


echo Building Win32 Help files
start /wait hcw /C /E /M "hlp\PsiSiGen.hpj"
if errorlevel 1 goto :Error
if not exist "hlp\PsiSiGen.hlp" goto :Error
if not exist "hlp\PsiSiGen.cnt" goto :Error
echo.
if exist Debug\nul copy "hlp\PsiSiGen.hlp" Debug
if exist Debug\nul copy "hlp\PsiSiGen.cnt" Debug
if exist Release\nul copy "hlp\PsiSiGen.hlp" Release
if exist Release\nul copy "hlp\PsiSiGen.cnt" Release
echo.
goto :done

:Error
echo hlp\PsiSiGen.hpj(1) : error: Problem encountered creating help file

:done
echo.
