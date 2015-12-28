// FaceLoginDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "FacePad.h"
#include "FaceLoginDlg.h"


// CFaceLoginDlg �Ի���

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


// CFaceLoginDlg ��Ϣ�������

BOOL CFaceLoginDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	return TRUE;  // return TRUE unless you set the focus to a control
	// �쳣: OCX ����ҳӦ���� FALSE
}

void CFaceLoginDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
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
