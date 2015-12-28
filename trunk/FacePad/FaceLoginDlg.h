#pragma once


// CFaceLoginDlg 对话框

class CFaceLoginDlg : public CDialog
{
	DECLARE_DYNAMIC(CFaceLoginDlg)

public:
	CFaceLoginDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CFaceLoginDlg();

// 对话框数据
	enum { IDD = IDD_FACELOGINDLG };

protected:
	HICON m_hIcon;

	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	//
};
