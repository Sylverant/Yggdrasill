/*
	This file is part of Yggdrasill
	Copyright (C) 2012, 2013, 2025 Lawrence Sebald

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License version 3 as
	published by  the Free Software Foundation.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "pch.h"
#include "framework.h"
#include "Yggdrasill.h"
#include "YggdrasillDlg.h"
#include "SettingsDialog.h"
#include "afxdialogex.h"

#include <detours/detours.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static LSTATUS OpenPSORegKey(HKEY &key) {
	return RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\SonicTeam\\PSOV2"), 0, KEY_READ | KEY_WRITE, &key);
}

static LSTATUS GetInstallDir(HKEY regKey, tstring &out) {
	WCHAR buf[MAX_PATH + 1];
	DWORD sz = sizeof(buf);
	LSTATUS err;

	err = RegQueryValueEx(regKey, _T("Dir"), 0, NULL, (LPBYTE)buf, &sz);
	if(err == ERROR_SUCCESS)
		out = buf;

	return err;
}

static LSTATUS SetCTRLFLAG1(HKEY regKey, bool online) {
	DWORD val;
	DWORD sz = sizeof(val);
	DWORD valType;
	LSTATUS err;

	err = RegQueryValueEx(regKey, _T("CTRLFLAG1"), 0, &valType, (LPBYTE)&val, &sz);
	if(err == ERROR_SUCCESS) {
		if(valType != REG_DWORD)
			return ERROR_APP_DATA_CORRUPT;

		if(online)
			val |= 0x00000010;
		else
			val &= 0xffffffef;

		err = RegSetValueEx(regKey, _T("CTRLFLAG1"), 0, REG_DWORD, (const BYTE*)&val, sz);
	}

	return err;
}

static std::string GetMithosLocation() {
	CHAR path[MAX_PATH];
	DWORD len = GetModuleFileNameA(NULL, path, MAX_PATH);

	PathRemoveFileSpecA(path);

	std::string dest = path;
	return dest + "\\Mithos.dll";
}

class CAboutDlg : public CDialog
{
	public:
		CAboutDlg();

#ifdef AFX_DESIGN_TIME
		enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
		virtual void DoDataExchange(CDataExchange* pDX);

		DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(IDD_ABOUTBOX) {
}

void CAboutDlg::DoDataExchange(CDataExchange *pDX) {
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


CYggdrasillDlg::CYggdrasillDlg(CWnd* pParent) :
	CDialog(IDD_YGGDRASILL_DIALOG, pParent) {
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CYggdrasillDlg::DoDataExchange(CDataExchange *pDX) {
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CYggdrasillDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_EXIT_BUTTON, &CYggdrasillDlg::OnBnClickedExitButton)
	ON_BN_CLICKED(IDC_ABOUT_BUTTON, &CYggdrasillDlg::OnBnClickedAboutButton)
	ON_BN_CLICKED(IDC_PSO_OPTS_BUTTON, &CYggdrasillDlg::OnBnClickedPsoOptsButton)
	ON_BN_CLICKED(IDC_SETTINGS_BUTTON, &CYggdrasillDlg::OnBnClickedSettingsButton)
	ON_BN_CLICKED(IDC_ONLINE_BUTTON, &CYggdrasillDlg::OnBnClickedOnlineButton)
	ON_BN_CLICKED(IDC_OFFLINE_BUTTON, &CYggdrasillDlg::OnBnClickedOfflineButton)
END_MESSAGE_MAP()


// CYggdrasillDlg message handlers

BOOL CYggdrasillDlg::OnInitDialog() {
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if(pSysMenu != nullptr) {
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if(!strAboutMenu.IsEmpty()) {
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CYggdrasillDlg::OnSysCommand(UINT nID, LPARAM lParam) {
	if((nID & 0xFFF0) == IDM_ABOUTBOX) {
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else {
		CDialog::OnSysCommand(nID, lParam);
	}
}

void CYggdrasillDlg::OnPaint() {
	if (IsIconic()) {
		CPaintDC dc(this);

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		dc.DrawIcon(x, y, m_hIcon);
	}
	else {
		CDialog::OnPaint();
	}
}

HCURSOR CYggdrasillDlg::OnQueryDragIcon() {
	return static_cast<HCURSOR>(m_hIcon);
}

void CYggdrasillDlg::OnBnClickedExitButton() {
	exit(0);
}

void CYggdrasillDlg::OnBnClickedAboutButton() {
	CAboutDlg dlgAbout;
	dlgAbout.DoModal();
}

void CYggdrasillDlg::OnBnClickedPsoOptsButton() {
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	LPTSTR cmd;
	HKEY key;
	tstring dir;
	tstring cmds;

	OpenPSORegKey(key);
	GetInstallDir(key, dir);
	RegCloseKey(key);

	cmds = _T("\"") + dir + _T("option.exe\" -autorun.exe");
	cmd = _tcsdup(cmds.c_str());

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	if(!CreateProcess(NULL, cmd, NULL, NULL, FALSE, 0, NULL, dir.c_str(), &si, &pi)) {
		MessageBox(_T("Failed to start option.exe!"));
		free(cmd);
		return;
	}

	/* Hide the launcher while the options dialog is running. */
	ShowWindow(SW_HIDE);
	WaitForSingleObject(pi.hProcess, INFINITE);
	ShowWindow(SW_SHOWNORMAL);

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	free(cmd);
}

void CYggdrasillDlg::OnBnClickedSettingsButton() {
	SettingsDialog dlg;
	INT_PTR nResponse = dlg.DoModal();

	if(nResponse == -1) {
		TRACE(traceAppMsg, 0, "Warning: dialog creation failed...\n");
	}
}

void CYggdrasillDlg::OnBnClickedOnlineButton() {
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	LPTSTR cmd;
	HKEY key;
	tstring dir;
	tstring cmds;
	DWORD dwFlags = CREATE_DEFAULT_ERROR_MODE | CREATE_SUSPENDED;

	OpenPSORegKey(key);
	GetInstallDir(key, dir);
	SetCTRLFLAG1(key, true);
	RegCloseKey(key);

	cmds = _T("\"") + dir + _T("pso.exe\" -online");
	cmd = _tcsdup(cmds.c_str());

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	if(!DetourCreateProcessWithDll(NULL, cmd, NULL, NULL, FALSE, dwFlags, NULL,
	                               dir.c_str(), &si, &pi,
	                               GetMithosLocation().c_str(), NULL)) {
		MessageBox(_T("Failed to start PSO!"));
		free(cmd);
		return;
	}

	/* Hide the launcher while the game is running. */
	ShowWindow(SW_HIDE);
	ResumeThread(pi.hThread);
	WaitForSingleObject(pi.hProcess, INFINITE);

	GetExitCodeProcess(pi.hProcess, &dwFlags);

	if(dwFlags > 128) {
		char tmp[1024];

		sprintf_s(tmp, 1024, "PSO Exited with Code %08x", dwFlags);
		MessageBoxA(NULL, tmp, "", 0);
	}

	Sleep(1000);

	ShowWindow(SW_SHOWNORMAL);

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	free(cmd);
}

void CYggdrasillDlg::OnBnClickedOfflineButton() {
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	LPTSTR cmd;
	HKEY key;
	tstring dir;
	tstring cmds;
	DWORD dwFlags = CREATE_DEFAULT_ERROR_MODE | CREATE_SUSPENDED;

	OpenPSORegKey(key);
	GetInstallDir(key, dir);
	SetCTRLFLAG1(key, false);
	RegCloseKey(key);

	cmds = _T("\"") + dir + _T("pso.exe\" -online");
	cmd = _tcsdup(cmds.c_str());

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	if(!DetourCreateProcessWithDll(NULL, cmd, NULL, NULL, FALSE, dwFlags, NULL,
		dir.c_str(), &si, &pi,
		GetMithosLocation().c_str(), NULL)) {
		MessageBox(_T("Failed to start PSO!"));
		free(cmd);
		return;
	}

	/* Hide the launcher while the game is running. */
	ShowWindow(SW_HIDE);
	ResumeThread(pi.hThread);
	WaitForSingleObject(pi.hProcess, INFINITE);

	GetExitCodeProcess(pi.hProcess, &dwFlags);

	if(dwFlags > 128) {
		char tmp[1024];

		sprintf_s(tmp, 1024, "PSO Exited with Code %08x", dwFlags);
		MessageBoxA(NULL, tmp, "", 0);
	}

	Sleep(1000);

	ShowWindow(SW_SHOWNORMAL);

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	free(cmd);
}
