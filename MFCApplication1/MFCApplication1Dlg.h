
// MFCApplication1Dlg.h: 헤더 파일
//

#pragma once
#include "opencv2/opencv.hpp" 
#include <opencv2/highgui/highgui.hpp>
#include "ProtocolDef.h"
#include "Common/NetworkManager.h"

using namespace cv;
using namespace std;
// CMFCApplication1Dlg 대화 상자

class CMFCApplication1Dlg : public CDialogEx
{
// 생성입니다.
public:
	CMFCApplication1Dlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MFCAPPLICATION1_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;
	CStatic m_picture;
	CButton m_btnLogin;
	CButton m_btnLogout;
	CButton m_btnStart;
	CButton m_btnAdd;
	CButton m_btnSelect;
	UINT m_radioBtnSecureMode;
	UINT m_radioBtnOperMode;
	CButton m_radioLearning;
	CButton m_radioRun;
	CButton m_radioTestRun;
	CButton m_radioSecure;
	CButton m_radioNonSecure;
	CEdit m_EditID;
	CEdit m_EditPW;
	CEdit m_EditName;
	CEdit m_EditImageNum;
	CEdit m_EditLog;
	CSpinButtonCtrl m_spinIMGNum;
	CListBox m_ListVideo;
	UINT mLearningCnt;
	
	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	bool m_bModeStart;
	CWinThread* m_pThread;
	bool m_isWorkingThread;
	Mat m_matImage;
	BITMAPINFO* m_pBitmapInfo;
	bool mShowReconnectMsg;
public:	

	afx_msg void OnDestroy();
	afx_msg void OnClose();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	
	afx_msg void OnBnClickedButtonLogin();
	afx_msg void OnBnClickedButtonModeApply();
	afx_msg void OnBnClickedButtonAddNewUser();
	afx_msg void OnBnClickSecureRadioCtrl(UINT ID);
	afx_msg void OnBnClickOperModeRadioCtrl(UINT ID);
	afx_msg void OnDeltaposSpinImageNum(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedButtonSelectVideo();

	void CreateBitmapInfo(int w, int h, int bpp);
	void DrawImage();
	void printLog(CString logStr);
	bool checkString(CString str);
	bool checkIDPW(CString str);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	LRESULT recvUserMsg(WPARAM wParam, LPARAM IParam);
	int getRadioBtnSecureMode();
	int getRadioBtnOperationMode();
	bool getPlayStatus();
	BITMAPINFO* getBitmapInfo();
	NetworkManager* getNetworkManager();
	bool IsWorkingThread();
	Mat* getMatImage();
	void clearVideoList();
	void setModeRadioBtnStatus();
	void resetStatus();

private:
	NetworkManager* mNetworkManager;
public:
	afx_msg void OnBnClickedButtonLogout();
};
