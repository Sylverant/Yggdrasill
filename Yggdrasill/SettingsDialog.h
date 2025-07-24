#pragma once
#include "afxdialogex.h"


// SettingsDialog dialog

class SettingsDialog : public CDialog
{
	DECLARE_DYNAMIC(SettingsDialog)

public:
	SettingsDialog(CWnd* pParent = nullptr);   // standard constructor
	virtual ~SettingsDialog();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SETTINGS_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
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
};
