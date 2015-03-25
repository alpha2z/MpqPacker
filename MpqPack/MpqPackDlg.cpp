// MpqPackDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "MpqPack.h"
#include "MpqPackDlg.h"
#include "MPQPackage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CMpqPackDlg �Ի���




CMpqPackDlg::CMpqPackDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMpqPackDlg::IDD, pParent)
	, m_packerState(PackerState_None)
	, m_sPath(_T(""))
	, m_sMpqPack(_T(""))
	, m_bConfig(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMpqPackDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS, m_progress);
	DDX_Text(pDX, IDC_EDIT_PATH, m_sPath);
	DDX_Text(pDX, IDC_EDIT_MPQ_PACK, m_sMpqPack);
	DDV_MaxChars(pDX, m_sPath, 260);
	DDV_MaxChars(pDX, m_sMpqPack, 260);
	DDX_Check(pDX, IDC_CHECK_CONFIG, m_bConfig);
}

BEGIN_MESSAGE_MAP(CMpqPackDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BTN_PACK, &CMpqPackDlg::OnBnClickedBtnPack)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BTN_COMPARE, &CMpqPackDlg::OnBnClickedBtnCompare)
	ON_BN_CLICKED(IDC_BTN_PATCH, &CMpqPackDlg::OnBnClickedBtnPatch)
	ON_BN_CLICKED(IDC_CHECK_CONFIG, &CMpqPackDlg::OnBnClickedCheckConfig)
END_MESSAGE_MAP()


// CMpqPackDlg ��Ϣ�������

BOOL CMpqPackDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	SetTimer(0,10,NULL);

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CMpqPackDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CMpqPackDlg::OnPaint()
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

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CMpqPackDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CMpqPackDlg::OnBnClickedBtnPack()
{
	CString sPath;
	CString sMpqPack;
	CString sConfig;
	GetDlgItemText(IDC_EDIT_PATH,sPath);
	GetDlgItemText(IDC_EDIT_MPQ_PACK,sMpqPack);
	GetDlgItemText(IDC_EDIT_CONFIG,sConfig);

	if ( sPath.IsEmpty() || sMpqPack.IsEmpty() )
	{
		return ;
	}
	// TODO: �ڴ���ӿؼ�֪ͨ����������
 	if( !m_packer.isBusy() && m_packerState == PackerState_None )
 	{
		if ( !m_bConfig )
		{
			m_packer.pack(sPath.GetBuffer(),sMpqPack.GetBuffer());
		}
 		else
		{
			if ( sConfig.IsEmpty() )
			{
				return;
			}
			m_packer.packWithConfig(sPath.GetBuffer(),sConfig.GetBuffer(),sMpqPack.GetBuffer());
		}
 
 		m_packerState = PackerState_Pack;
 	}
}

void CMpqPackDlg::OnBnClickedBtnCompare()
{
	CString sMpq;
	CString sMpqCompare;
	CString sMpqDiff;
	GetDlgItemText(IDC_EDIT_MPQ_SRC,sMpq);
	GetDlgItemText(IDC_EDIT_MPQ_DES,sMpqCompare);
	GetDlgItemText(IDC_EDIT_MPQ_DIFF,sMpqDiff);

	if ( sMpq.IsEmpty() || sMpqCompare.IsEmpty() || sMpqDiff.IsEmpty() )
	{
		return ;
	}
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if( !m_differ.isBusy() && m_packerState == PackerState_None )
	{
		m_differ.compare(sMpq.GetBuffer(),sMpqCompare.GetBuffer(),sMpqDiff.GetBuffer());

		m_packerState = PackerState_Compare;
	}
}

void CMpqPackDlg::OnBnClickedBtnPatch()
{
	CString sMpq;
	CString sMpqPatch;
	GetDlgItemText(IDC_EDIT_MPQ,sMpq);
	GetDlgItemText(IDC_EDIT_MPQ_PATCH,sMpqPatch);

	if ( sMpq.IsEmpty() || sMpqPatch.IsEmpty() )
	{
		return ;
	}
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if( !m_patcher.isBusy() && m_packerState == PackerState_None )
	{
		m_patcher.patch(sMpq.GetBuffer(),sMpqPatch.GetBuffer(),"11111.bak");

		m_packerState = PackerState_Patch;
	}
}

void CMpqPackDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ

	switch(m_packerState)
	{
	case PackerState_None:
		{
			m_progress.SetRange(0,100);
			m_progress.SetPos(0);
		}
		break;
	case PackerState_Pack:
		{
			if ( m_packer.isBusy() )
			{
				m_progress.SetRange(0,m_packer.getRange());
				m_progress.SetPos(m_packer.getPos());
			}
			else
			{
				m_packerState = PackerState_None;
				if ( m_packer.getInfo().empty() )
				{
					MessageBox("pack complete��");
				}
				else
				{
					MessageBox("pack failed��");
				}
			}

		}
		break;
	case PackerState_Compare:
		{
			if ( m_differ.isBusy() )
			{
				m_progress.SetRange(0,m_differ.getRange());
				m_progress.SetPos(m_differ.getPos());
			}
			else
			{
				m_packerState = PackerState_None;
				if ( m_differ.getInfo().empty() )
				{
					MessageBox("compare complete��");
				}
				else
				{
					MessageBox("compare failed��");
				}
			}

		}
		break;
	case PackerState_Patch:
		{
			if ( m_patcher.isBusy() )
			{
				m_progress.SetRange(0,m_patcher.getRange());
				m_progress.SetPos(m_patcher.getPos());
			}
			else
			{
				m_packerState = PackerState_None;
				if ( m_patcher.getInfo().empty() )
				{
					MessageBox("patch complete��");
				}
				else
				{
					MessageBox("patch failed��");
				}
			}

		}
		break;
	}

	CDialog::OnTimer(nIDEvent);
}

void CMpqPackDlg::OnBnClickedCheckConfig()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData();

	if ( m_bConfig )
	{
		((CEdit*)GetDlgItem(IDC_EDIT_CONFIG))->EnableWindow(TRUE);
	}
	else
	{
		((CEdit*)GetDlgItem(IDC_EDIT_CONFIG))->EnableWindow(FALSE);
	}
}
