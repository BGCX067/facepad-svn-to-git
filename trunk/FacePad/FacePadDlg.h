// FacePadDlg.h : ͷ�ļ�
//

#pragma once

#include "..\vtOS\Shell\Os\vtWindowsUI.h"


// CFacePadDlg �Ի���
class CFacePadDlg : public CDialog
{
// ����
public:
	CFacePadDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_FACEPAD_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	DECLARE_MESSAGE_MAP()

public:
	vtWindowsUI m_vtWindowsUI;

private:
	//	
};
