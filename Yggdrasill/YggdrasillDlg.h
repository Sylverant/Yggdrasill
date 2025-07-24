
// YggdrasillDlg.h : header file
//

#pragma once


// CYggdrasillDlg dialog
class CYggdrasillDlg : public CDialog
{
// Construction
public:
	CYggdrasillDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_YGGDRASILL_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedExitButton();
	afx_msg void OnBnClickedAboutButton();
	afx_msg void OnBnClickedPsoOptsButton();
	afx_msg void OnBnClickedSettingsButton();
	afx_msg void OnBnClickedOnlineButton();
	afx_msg void OnBnClickedOfflineButton();
};
