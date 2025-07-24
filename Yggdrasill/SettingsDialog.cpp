/*
	This file is part of Yggdrasill
	Copyright (C) 2025 Lawrence Sebald

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
#include "Yggdrasill.h"
#include "afxdialogex.h"
#include "SettingsDialog.h"

IMPLEMENT_DYNAMIC(SettingsDialog, CDialog)

SettingsDialog::SettingsDialog(CWnd *pParent) :
	CDialog(IDD_SETTINGS_DIALOG, pParent), m_displayMode(0) {
}

SettingsDialog::~SettingsDialog() {
}

void SettingsDialog::DoDataExchange(CDataExchange *pDX) {
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SETTING_DISABLEIME, m_disableime);
	DDX_Control(pDX, IDC_SETTING_SERVERADDR, m_serverAddress);
	DDX_Control(pDX, IDC_SETTING_SERVER, m_server);
	DDX_Control(pDX, IDC_SETTING_WINHEIGHT, m_windowHeight);
	DDX_Control(pDX, IDC_SETTING_WINWIDTH, m_windowWidth);
	DDX_Control(pDX, IDC_SETTING_CUSSHACK, m_cusshack);
	DDX_Control(pDX, IDC_SETTING_MAPFIX, m_mapfix);
	DDX_Control(pDX, IDC_SETTING_V1NAME, m_v1name);
	DDX_Control(pDX, IDC_SETTING_MUSICPATCH, m_musicpatch);
	DDX_Control(pDX, IDC_SETTING_MOUSECURSOR, m_showMouse);
	DDX_Radio(pDX, IDC_SETTING_FULLSCREEN, m_displayMode);
}

BOOL SettingsDialog::OnInitDialog() {
	CDialog::OnInitDialog();

	CWinApp *app = AfxGetApp();
	CString addr = app->GetProfileString(_T("Mithos"), _T("ServerAddr"), _T("sylverant.net"));
	
	if(addr.Compare(_T("sylverant.net")) == 0)
		m_server.SetCurSel(m_server.FindStringExact(0, _T("Sylverant")));
	else if(addr.Compare(_T("psobb.dyndns.org")) == 0)
		m_server.SetCurSel(m_server.FindStringExact(0, _T("Schtserv")));
	else {
		m_server.SetCurSel(m_server.FindStringExact(0, _T("Other")));
		m_serverAddress.SetWindowTextW(addr.GetString());
		m_serverAddress.EnableWindow(TRUE);
	}

	m_disableime.SetCheck(!!app->GetProfileInt(_T("Mithos"), _T("DisableIME"), 1));
	m_v1name.SetCheck(!!app->GetProfileInt(_T("Mithos"), _T("V1Names"), 1));
	m_cusshack.SetCheck(!!app->GetProfileInt(_T("Mithos"), _T("Cusshack"), 0));
	m_mapfix.SetCheck(!!app->GetProfileInt(_T("Mithos"), _T("MapFix"), 1));
	m_musicpatch.SetCheck(!!app->GetProfileInt(_T("Mithos"), _T("MusicPatch"), 0));
	m_showMouse.SetCheck(!!app->GetProfileInt(_T("Mithos"), _T("ShowMouse"), 0));

	m_displayMode = app->GetProfileInt(_T("Mithos"), _T("DisplayMode"), 0);

	if(m_displayMode == 3) {
		m_windowHeight.EnableWindow(TRUE);
		m_windowWidth.EnableWindow(TRUE);
	}

	CString tmp;
	tmp.Format(_T("%d"), app->GetProfileInt(_T("Mithos"), _T("WindowHeight"), 960));
	m_windowHeight.SetWindowText(tmp.GetString());
	tmp.Format(_T("%d"), app->GetProfileInt(_T("Mithos"), _T("WindowWidth"), 1280));
	m_windowWidth.SetWindowText(tmp.GetString());

	UpdateData(FALSE);

	return TRUE;
}

BEGIN_MESSAGE_MAP(SettingsDialog, CDialog)
	ON_CBN_SELCHANGE(IDC_SETTING_SERVER, &SettingsDialog::OnCbnSelchangeSettingServer)
	ON_BN_CLICKED(IDOK, &SettingsDialog::OnBnClickedOk)
	ON_BN_CLICKED(IDC_SETTING_FULLSCREEN, &SettingsDialog::OnClickedSettingFullscreen)
	ON_BN_CLICKED(IDC_SETTING_BORDERLESS, &SettingsDialog::OnClickedSettingBorderless)
	ON_BN_CLICKED(IDC_SETTING_WINDOWED, &SettingsDialog::OnClickedSettingWindowed)
	ON_BN_CLICKED(IDC_SETTING_BORDERLESS_STRETCH, &SettingsDialog::OnClickedSettingBorderlessStretch)
END_MESSAGE_MAP()


void SettingsDialog::OnCbnSelchangeSettingServer() {
	int other = m_server.FindStringExact(0, _T("Other"));
	m_serverAddress.EnableWindow(m_server.GetCurSel() == other);
}

void SettingsDialog::OnBnClickedOk() {
	int srv = m_server.GetCurSel();
	CWinApp *app = AfxGetApp();

	if(srv == m_server.FindStringExact(0, _T("Sylverant"))) {
		app->WriteProfileString(_T("Mithos"), _T("ServerAddr"), _T("sylverant.net"));
	}
	else if(srv == m_server.FindStringExact(0, _T("Schtserv"))) {
		app->WriteProfileString(_T("Mithos"), _T("ServerAddr"), _T("psobb.dyndns.org"));
	}
	else {
		CString addr;
		m_serverAddress.GetWindowText(addr);
		app->WriteProfileString(_T("Mithos"), _T("ServerAddr"), addr.GetString());
	}

	app->WriteProfileInt(_T("Mithos"), _T("DisableIME"), !!m_disableime.GetCheck());
	app->WriteProfileInt(_T("Mithos"), _T("V1Names"), !!m_v1name.GetCheck());
	app->WriteProfileInt(_T("Mithos"), _T("Cusshack"), !!m_cusshack.GetCheck());
	app->WriteProfileInt(_T("Mithos"), _T("MapFix"), !!m_mapfix.GetCheck());
	app->WriteProfileInt(_T("Mithos"), _T("MusicPatch"), !!m_musicpatch.GetCheck());
	app->WriteProfileInt(_T("Mithos"), _T("ShowMouse"), !!m_showMouse.GetCheck());
	app->WriteProfileInt(_T("Mithos"), _T("DisplayMode"), m_displayMode);

	CString tmp;
	m_windowWidth.GetWindowText(tmp);
	app->WriteProfileInt(_T("Mithos"), _T("WindowWidth"), _ttoi(tmp.GetString()));
	m_windowHeight.GetWindowText(tmp);
	app->WriteProfileInt(_T("Mithos"), _T("WindowHeight"), _ttoi(tmp.GetString()));

	CDialog::OnOK();
}

void SettingsDialog::OnClickedSettingFullscreen() {
	UpdateData(TRUE);
	m_windowHeight.EnableWindow(FALSE);
	m_windowWidth.EnableWindow(FALSE);
}

void SettingsDialog::OnClickedSettingBorderless() {
	UpdateData(TRUE);
	m_windowHeight.EnableWindow(FALSE);
	m_windowWidth.EnableWindow(FALSE);
}

void SettingsDialog::OnClickedSettingBorderlessStretch() {
	UpdateData(TRUE);
	m_windowHeight.EnableWindow(FALSE);
	m_windowWidth.EnableWindow(FALSE);
}

void SettingsDialog::OnClickedSettingWindowed() {
	UpdateData(TRUE);
	m_windowHeight.EnableWindow(TRUE);
	m_windowWidth.EnableWindow(TRUE);
}
