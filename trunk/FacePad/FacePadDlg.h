// FacePadDlg.h : 头文件
//

#pragma once

#include "..\vtOS\Shell\Os\vtWindowsUI.h"


// CFacePadDlg 对话框
class CFacePadDlg : public CDialog
{
// 构造
public:
	CFacePadDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_FACEPAD_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
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
