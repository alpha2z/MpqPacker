// MpqPackDlg.h : 头文件
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

// CMpqPackDlg 对话框
class CMpqPackDlg : public CDialog
{
// 构造
public:
	CMpqPackDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_MPQPACK_DIALOG };

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
