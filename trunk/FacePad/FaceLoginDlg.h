#pragma once


// CFaceLoginDlg �Ի���

class CFaceLoginDlg : public CDialog
{
	DECLARE_DYNAMIC(CFaceLoginDlg)

public:
	CFaceLoginDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CFaceLoginDlg();

// �Ի�������
	enum { IDD = IDD_FACELOGINDLG };

protected:
	HICON m_hIcon;

	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	//
};
