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

class SNAKDialog : public CDialog {
    DECLARE_DYNAMIC(SNAKDialog)

    public:
        SNAKDialog(CWnd *pParent = nullptr);
        virtual ~SNAKDialog();

#ifdef AFX_DESIGN_TIME
        enum { IDD = IDD_SNAK_DIALOG };
#endif

    protected:
        virtual void DoDataExchange(CDataExchange *pDX);

        DECLARE_MESSAGE_MAP()

    public:
        CEdit m_accessKey;
        CEdit m_email;
        CEdit m_serial;

        virtual BOOL OnInitDialog();
        afx_msg void OnBnClickedOk();
};
