// FaceLoginDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "FacePad.h"
#include "FaceLoginDlg.h"


// CFaceLoginDlg 对话框

IMPLEMENT_DYNAMIC(CFaceLoginDlg, CDialog)

CFaceLoginDlg::CFaceLoginDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFaceLoginDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CFaceLoginDlg::~CFaceLoginDlg()
{
}

void CFaceLoginDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CFaceLoginDlg, CDialog)
	ON_WM_QUERYDRAGICON()
	ON_WM_PAINT()
END_MESSAGE_MAP()


// CFaceLoginDlg 消息处理程序

BOOL CFaceLoginDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CFaceLoginDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

HCURSOR CFaceLoginDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
	//return CDialog::OnQueryDragIcon();
}
