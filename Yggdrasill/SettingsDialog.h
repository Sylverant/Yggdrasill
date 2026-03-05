/*
    This file is part of Yggdrasill
    Copyright (C) 2026 Lawrence Sebald

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

#pragma once
#include "afxdialogex.h"

class SettingsDialog : public CDialog {
    DECLARE_DYNAMIC(SettingsDialog)

    public:
        SettingsDialog(CWnd* pParent = nullptr);
        virtual ~SettingsDialog();

#ifdef AFX_DESIGN_TIME
        enum { IDD = IDD_SETTINGS_DIALOG };
#endif

    protected:
        virtual void DoDataExchange(CDataExchange* pDX);
        virtual BOOL OnInitDialog();

        DECLARE_MESSAGE_MAP()
    public:
        CButton m_disableime;
        CEdit m_serverAddress;
        CComboBox m_server;
        CEdit m_windowHeight;
        CEdit m_windowWidth;
        CButton m_cusshack;
        CButton m_mapfix;
        CButton m_v1name;
        afx_msg void OnCbnSelchangeSettingServer();
        afx_msg void OnBnClickedOk();

        CButton m_musicpatch;
        CButton m_showMouse;
        int m_displayMode;
        afx_msg void OnClickedSettingFullscreen();
        afx_msg void OnClickedSettingBorderless();
        afx_msg void OnClickedSettingWindowed();
        afx_msg void OnClickedSettingBorderlessStretch();
        afx_msg void OnBnClickedSnakButton();
};
