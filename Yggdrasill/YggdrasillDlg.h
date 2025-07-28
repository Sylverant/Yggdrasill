#pragma once

class CYggdrasillDlg : public CDialog {
    public:
        CYggdrasillDlg(CWnd* pParent = nullptr);

#ifdef AFX_DESIGN_TIME
        enum { IDD = IDD_YGGDRASILL_DIALOG };
#endif

    protected:
        virtual void DoDataExchange(CDataExchange* pDX);
        HICON m_hIcon;

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
