// MpqPackDlg.h : ͷ�ļ�
//

#pragma once

#include "MPQPacker.h"
#include "MPQPatcher.h"
#include "MPQDiffer.h"
#include "afxcmn.h"

enum PackerState{
	PackerState_None,
	PackerState_Pack,
	PackerState_Compare,
	PackerState_Patch,
};

// CMpqPackDlg �Ի���
class CMpqPackDlg : public CDialog
{
// ����
public:
	CMpqPackDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_MPQPACK_DIALOG };

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
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnPack();
	afx_msg void OnBnClickedBtnCompare();
	afx_msg void OnBnClickedBtnPatch();
	afx_msg void OnTimer(UINT_PTR nIDEvent);

protected:
	PackerState m_packerState;
	MPQPacker	m_packer;
	MPQDiffer	m_differ;
	MPQPatcher	m_patcher;

public:
	CProgressCtrl m_progress;
	CString m_sPath;
	CString m_sMpqPack;
	BOOL m_bConfig;
	afx_msg void OnBnClickedCheckConfig();
};
