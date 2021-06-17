
// MFCApplication1Dlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "MFCApplication1.h"
#include "MFCApplication1Dlg.h"
#include "afxdialogex.h"
#include "locale.h"
#include <regex>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#define REQ_TIMEOUT_TIMER 3001

// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMFCApplication1Dlg 대화 상자

UINT ThreadForRecv(LPVOID param)
{
	CMFCApplication1Dlg* pMain = (CMFCApplication1Dlg*)param;
	if (pMain == NULL) return 0;
	NetworkManager* networkManager = pMain->getNetworkManager();
	Mat* image = pMain->getMatImage();
	if (networkManager == NULL || image == NULL) return 0;

	while (pMain->IsWorkingThread())
	{
		Sleep(0);	
		// KSS		
		bool retvalue = false;
		retvalue = networkManager->get_a_packet(image);
		if (retvalue && pMain->getPlayStatus())
		{
			pMain->CreateBitmapInfo(image->cols, image->rows, image->channels() * 8);
			pMain->DrawImage();
		}
	}

	return 0;
}

CMFCApplication1Dlg::CMFCApplication1Dlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MFCAPPLICATION1_DIALOG, pParent)
	, m_radioBtnSecureMode(0)
	, m_radioBtnOperMode(0)
	, m_bModeStart(false)
	, m_isWorkingThread(false)
	, m_pThread(NULL)
	, mNetworkManager(NULL)
	, m_pBitmapInfo(NULL)
	, mLearningCnt(0)
	, mShowReconnectMsg(false)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCApplication1Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PICTURE, m_picture);
	DDX_Control(pDX, IDC_BUTTON_MODE_START, m_btnStart); 
	DDX_Control(pDX, IDC_BUTTON_ADD_NEW_USER, m_btnAdd);
	DDX_Control(pDX, IDC_RADIO_MODE_LEARNING, m_radioLearning);
	DDX_Control(pDX, IDC_RADIO_MODE_RUN, m_radioRun);
	DDX_Control(pDX, IDC_RADIO_MODE_TEST_RUN, m_radioTestRun); 
	DDX_Control(pDX, IDC_RADIO_SECURE, m_radioSecure);
	DDX_Control(pDX, IDC_RADIO_NON_SECURE, m_radioNonSecure);
	DDX_Control(pDX, IDC_EDIT_NEW_USER_NAME, m_EditName);
	DDX_Control(pDX, IDC_EDIT_NEW_USER_IMGNUM, m_EditImageNum);
	DDX_Control(pDX, IDC_EDIT_INPUT_ID, m_EditID);
	DDX_Control(pDX, IDC_EDIT_INPUT_PW, m_EditPW);
	DDX_Control(pDX, IDC_EDIT_LOG, m_EditLog);
	DDX_Radio(pDX, IDC_RADIO_SECURE, (int&)m_radioBtnSecureMode);
	DDX_Radio(pDX, IDC_RADIO_MODE_RUN, (int&)m_radioBtnOperMode);
	DDX_Control(pDX, IDC_SPIN_IMAGE_NUM, m_spinIMGNum);
	DDX_Control(pDX, IDC_LIST_VIDEO, m_ListVideo);
	DDX_Control(pDX, IDC_BUTTON_SELECT_VIDEO, m_btnSelect);
	DDX_Control(pDX, IDC_BUTTON_LOGIN, m_btnLogin);
	DDX_Control(pDX, IDC_BUTTON_LOGOUT, m_btnLogout); 
	DDX_Control(pDX, IDC_IPADDRESS1, m_IPEdit);
}

BEGIN_MESSAGE_MAP(CMFCApplication1Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_LOGIN, &CMFCApplication1Dlg::OnBnClickedButtonLogin)
	ON_BN_CLICKED(IDC_BUTTON_MODE_START, &CMFCApplication1Dlg::OnBnClickedButtonModeApply)
	ON_BN_CLICKED(IDC_BUTTON_ADD_NEW_USER, &CMFCApplication1Dlg::OnBnClickedButtonAddNewUser)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_RADIO_SECURE, IDC_RADIO_NON_SECURE, &CMFCApplication1Dlg::OnBnClickSecureRadioCtrl)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_RADIO_MODE_RUN, IDC_RADIO_MODE_TEST_RUN, &CMFCApplication1Dlg::OnBnClickOperModeRadioCtrl)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_IMAGE_NUM, &CMFCApplication1Dlg::OnDeltaposSpinImageNum)
	ON_MESSAGE(MESSAGE_USER, &CMFCApplication1Dlg::recvUserMsg)
	ON_BN_CLICKED(IDC_BUTTON_SELECT_VIDEO, &CMFCApplication1Dlg::OnBnClickedButtonSelectVideo)
	ON_BN_CLICKED(IDC_BUTTON_LOGOUT, &CMFCApplication1Dlg::OnBnClickedButtonLogout)
END_MESSAGE_MAP()


// CMFCApplication1Dlg 메시지 처리기

BOOL CMFCApplication1Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	setlocale(LC_ALL, "");
	m_IPEdit.SetAddress(192, 168, 0, 116);
	m_EditImageNum.SetWindowText(_T("5"));
	m_spinIMGNum.SetRange(5, 8);
	m_spinIMGNum.SetPos(1);
	m_radioLearning.SetCheck(FALSE);
	m_radioRun.SetCheck(TRUE);
	m_EditID.SetLimitText(10);
	m_EditPW.SetLimitText(10);
	m_EditName.SetLimitText(10);
	mNetworkManager = new NetworkManager();

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CMFCApplication1Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CMFCApplication1Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CMFCApplication1Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CMFCApplication1Dlg::setIPAdress()
{
	CString ipStr;
	m_IPEdit.GetWindowText(ipStr);
	string str = string(CT2CA(ipStr));
	mNetworkManager->setIPAdress(str);
}



void CMFCApplication1Dlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	if (m_pThread != NULL)
	{
		m_isWorkingThread = false;
		m_pThread = NULL;
	}
}


void CMFCApplication1Dlg::OnClose()
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	delete mNetworkManager;
	delete m_pBitmapInfo;
	CDialogEx::OnClose();
}


void CMFCApplication1Dlg::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent)
	{
	case REQ_TIMEOUT_TIMER:
		if (mNetworkManager->requestType() == MSG_LOGIN)
			SendMessage(MESSAGE_USER, MSG_LOGIN_FAIL, NULL);
		else if(mNetworkManager->requestType() == MSG_CONTROL_MODE)
			SendMessage(MESSAGE_USER, MSG_MODE_CHANGE_FAIL, NULL);
		else if (mNetworkManager->requestType() == MSG_START_LEARNING_MODE)
			SendMessage(MESSAGE_USER, MSG_LEARNING_FAIL, NULL);
		else if (mNetworkManager->requestType() == MSG_VIDEO_SELECT)
			SendMessage(MESSAGE_USER, MSG_VIDEO_SELECTED_FAIL, NULL);
		KillTimer(REQ_TIMEOUT_TIMER);
		break;
	}

	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	CDialogEx::OnTimer(nIDEvent);
}

void CMFCApplication1Dlg::CreateBitmapInfo(int w, int h, int bpp)
{
	if (bpp == 8)
		m_pBitmapInfo = (BITMAPINFO*) new BYTE[sizeof(BITMAPINFO) + 255 * sizeof(RGBQUAD)];
	else // 24 or 32bit
		m_pBitmapInfo = (BITMAPINFO*) new BYTE[sizeof(BITMAPINFO)];

	m_pBitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_pBitmapInfo->bmiHeader.biPlanes = 1;
	m_pBitmapInfo->bmiHeader.biBitCount = bpp;
	m_pBitmapInfo->bmiHeader.biCompression = BI_RGB;
	m_pBitmapInfo->bmiHeader.biSizeImage = 0;
	m_pBitmapInfo->bmiHeader.biXPelsPerMeter = 0;
	m_pBitmapInfo->bmiHeader.biYPelsPerMeter = 0;
	m_pBitmapInfo->bmiHeader.biClrUsed = 0;
	m_pBitmapInfo->bmiHeader.biClrImportant = 0;

	if (bpp == 8)
	{
		for (int i = 0; i < 256; i++)
		{
			m_pBitmapInfo->bmiColors[i].rgbBlue = (BYTE)i;
			m_pBitmapInfo->bmiColors[i].rgbGreen = (BYTE)i;
			m_pBitmapInfo->bmiColors[i].rgbRed = (BYTE)i;
			m_pBitmapInfo->bmiColors[i].rgbReserved = 0;
		}
	}

	m_pBitmapInfo->bmiHeader.biWidth = w;
	m_pBitmapInfo->bmiHeader.biHeight = -h;
}

void CMFCApplication1Dlg::DrawImage()
{
	CClientDC dc(GetDlgItem(IDC_PICTURE));

	CRect rect;
	GetDlgItem(IDC_PICTURE)->GetClientRect(&rect);

	SetStretchBltMode(dc.GetSafeHdc(), COLORONCOLOR);
	StretchDIBits(dc.GetSafeHdc(), 0, 0, rect.Width(), rect.Height(), 0, 0, m_matImage.cols, m_matImage.rows, m_matImage.data, m_pBitmapInfo, DIB_RGB_COLORS, SRCCOPY);
}

void CMFCApplication1Dlg::printLog(CString logStr)
{
	CTime time = CTime::GetCurrentTime();
	CString log;
	log.Format(_T("[%s] %s\r\n"), (LPCTSTR)time.Format(L"%H:%M:%S"), (LPCTSTR)logStr);
	int len = m_EditLog.GetWindowTextLength();
	m_EditLog.SetSel(len, len);
	m_EditLog.ReplaceSel(log);
}

bool CMFCApplication1Dlg::checkString(CString str)
{
	regex pattern("^[a-zA-Z]*$");
	if (regex_match(std::string(CT2CA(str)) , pattern))
		return true;
	else
		return false;
}

bool CMFCApplication1Dlg::checkIDPW(CString str)
{
	regex pattern("^[a-zA-Z0-9]*$");
	if (regex_match(std::string(CT2CA(str)), pattern))
		return true;
	else
		return false;
}

void CMFCApplication1Dlg::OnBnClickedButtonLogin()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString id;
	CString pw;
	m_EditID.GetWindowTextW(id);
	m_EditPW.GetWindowTextW(pw);
	if (id.IsEmpty() || checkIDPW(id) == false || pw.IsEmpty() || checkIDPW(pw) == false)
	{
		AfxMessageBox(_T("Please enter a valid ID and PW. (Alphabet, numeric only)"));
		return;
	}

	setIPAdress();

	m_btnLogin.EnableWindow(FALSE);
	m_radioSecure.EnableWindow(FALSE);
	m_radioNonSecure.EnableWindow(FALSE);
	m_EditID.EnableWindow(FALSE);
	m_EditPW.EnableWindow(FALSE);
	mNetworkManager->setSecureMode((m_radioBtnSecureMode == MODE_SECURE) ? true : false);
	
	// KSS TODO
	if (false == mNetworkManager->openTcpConnection())
	{
		printf(" Fail OpenTcpConnection\n");
		m_btnLogin.EnableWindow(TRUE);
		m_radioSecure.EnableWindow(TRUE);
		m_radioNonSecure.EnableWindow(TRUE);
		m_EditID.EnableWindow(TRUE);
		m_EditPW.EnableWindow(TRUE);
		AfxMessageBox(_T("Connection failed"));
		return;
	}	

	// send login ID, PW
	string ids = string(CT2CA(id));
	string pws = string(CT2CA(pw));
	CLoginProtocol login(ids, pws);
	mNetworkManager->send_packet(login);
	SetTimer(REQ_TIMEOUT_TIMER, 5000, NULL);
	// receive login result

	// login success
	m_isWorkingThread = true;
	m_pThread = AfxBeginThread(ThreadForRecv, this);

	//test
	//AfxMessageBox((mNetworkManager->isAdmin()) ? _T("로그인 성공! (Admin)") : _T("로그인 성공! (Normal user)"));
	//m_btnStart.EnableWindow(TRUE);
	//setModeRadioBtnStatus();
}

void CMFCApplication1Dlg::OnBnClickedButtonModeApply()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_bModeStart = !m_bModeStart;
	m_btnStart.SetWindowText((m_bModeStart) ? CString("Stop") : CString("Apply"));

	setModeRadioBtnStatus();

	if (m_bModeStart)//start
	{
		if (m_radioBtnOperMode == MODE_LEARNING)
		{
			m_EditName.EnableWindow(TRUE);
			m_EditImageNum.EnableWindow(TRUE);
			m_btnAdd.EnableWindow(TRUE);
			CControlModeProtocol mode(protocol_msg::ControlMode::LEARNING);
			mNetworkManager->send_packet(mode);
		}
		else if (m_radioBtnOperMode == MODE_TEST_RUN)
		{
			m_ListVideo.EnableWindow(TRUE);
			m_btnSelect.EnableWindow(TRUE);
			CControlModeProtocol mode(protocol_msg::ControlMode::TESTRUN);
			mNetworkManager->send_packet(mode);
		}
		else if (m_radioBtnOperMode == MODE_RUN)
		{
			CControlModeProtocol mode(protocol_msg::ControlMode::RUN);
			mNetworkManager->send_packet(mode);
		}
		m_btnStart.EnableWindow(FALSE);
		SetTimer(REQ_TIMEOUT_TIMER, 5000, NULL);
	}
	else//stop
	{
		clearVideoList();
		m_EditName.EnableWindow(FALSE);
		m_EditImageNum.EnableWindow(FALSE);
		m_btnAdd.EnableWindow(FALSE);
		m_ListVideo.EnableWindow(FALSE);
		m_btnSelect.EnableWindow(FALSE);
	}
}

void CMFCApplication1Dlg::OnBnClickedButtonAddNewUser()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	mLearningCnt = 0;
	CString name;
	CString num;
	m_EditName.GetWindowTextW(name);
	m_EditImageNum.GetWindowTextW(num);
	int cnt = _ttoi(num);
	if (name.IsEmpty() || checkString(name) == false)
	{
		AfxMessageBox(_T("Please enter a valid name. (Alphabet Only)"));
		return;
	}

	string namestr = string(CT2CA(name));
	CLearningModeProtocol learningReq(namestr, cnt);
	mNetworkManager->send_packet(learningReq);
	m_EditName.EnableWindow(false);
	m_EditImageNum.EnableWindow(false);
	m_btnAdd.EnableWindow(false);
	SetTimer(REQ_TIMEOUT_TIMER, cnt * 3500, NULL);//time out :20 sec
}

void CMFCApplication1Dlg::OnBnClickSecureRadioCtrl(UINT ID)
{
	UpdateData(true);
}

void CMFCApplication1Dlg::OnBnClickOperModeRadioCtrl(UINT ID)
{
	UpdateData(true);
}

void CMFCApplication1Dlg::OnDeltaposSpinImageNum(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	int val = pNMUpDown->iPos + pNMUpDown->iDelta;

	if ((5 <= val) && (val <= 8))
	{
		CString sValue;
		sValue.Format(_T("%d\n"), val);
		m_EditImageNum.SetWindowText(sValue);
	}

	*pResult = 0;
}

BOOL CMFCApplication1Dlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_ESCAPE)
			return TRUE;
		else if (pMsg->wParam == VK_RETURN)
			return TRUE;
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}

LRESULT CMFCApplication1Dlg::recvUserMsg(WPARAM wParam, LPARAM IParam)
{
	
	switch (wParam)
	{
	case MSG_VIDEO_FILE_LIST:
	{
		KillTimer(REQ_TIMEOUT_TIMER);
		m_btnStart.EnableWindow(TRUE);
		if (IParam == NULL) return LRESULT();
		CString* pstrString = (CString*)IParam;
		CString strString = pstrString->GetString();
		m_ListVideo.AddString(strString);
	}
	break;
	case MSG_LOGIN_SUCCESS:
		KillTimer(REQ_TIMEOUT_TIMER);
		mShowReconnectMsg = false;
		AfxMessageBox((mNetworkManager->isAdmin()) ? _T("Login success! (Admin)") : _T("Login success! (Normal user)"));
		if (!mNetworkManager->isAdmin())
		{
			m_radioTestRun.SetCheck(FALSE);
			m_radioLearning.SetCheck(FALSE);
			m_radioRun.SetCheck(TRUE);
			m_radioBtnOperMode = 0;
		}
		m_btnStart.EnableWindow(TRUE);
		m_btnLogout.EnableWindow(TRUE);
		setModeRadioBtnStatus();
		break;
	case MSG_LOGIN_FAIL:
		mNetworkManager->setRequestType(0);
		m_btnLogin.EnableWindow(TRUE);
		m_radioSecure.EnableWindow(TRUE);
		m_radioNonSecure.EnableWindow(TRUE);
		m_EditID.EnableWindow(TRUE);
		m_EditPW.EnableWindow(TRUE);
		m_isWorkingThread = false;
		resetStatus();
		mNetworkManager->closeTCPConnection();
		mNetworkManager->resetStatus();
		AfxMessageBox(_T("Login failed"));
		break;
	case MSG_MODE_CHANGE_SUCCESS:
		KillTimer(REQ_TIMEOUT_TIMER);
		m_btnStart.EnableWindow(TRUE);
		printLog(_T("Mode change success"));
		break;
	case MSG_MODE_CHANGE_FAIL:
		m_btnStart.EnableWindow(TRUE);
		printLog(_T("Mode change failed"));
		break;
	case MSG_LEARNING_SUCCESS:
	{
		mLearningCnt++;
		CString num;
		m_EditImageNum.GetWindowTextW(num);
		unsigned int cnt = _ttoi(num);
		if(mLearningCnt == 1)
			printLog(_T("LEARNING start"));
		else if (mLearningCnt > 1 && mLearningCnt < cnt + 2)
		{
			CString log;
			log.Format(L"LEARNING .....%d/%d", mLearningCnt-1, cnt);
			printLog(log);
		}
		
		if (cnt + 2 <= mLearningCnt)
		{
			KillTimer(REQ_TIMEOUT_TIMER);
			mLearningCnt = 0;
			m_btnAdd.EnableWindow(TRUE);
			m_EditName.EnableWindow(TRUE);
			m_EditImageNum.EnableWindow(TRUE);
			printLog(_T("LEARNING success"));
		}
	}
		break;
	case MSG_LEARNING_FAIL:
		m_btnAdd.EnableWindow(TRUE);
		m_EditName.EnableWindow(TRUE);
		m_EditImageNum.EnableWindow(TRUE);
		printLog(_T("LEARNING failed"));
		break;
	case MSG_VIDEO_SELECTED_SUCCESS:
		KillTimer(REQ_TIMEOUT_TIMER);
		m_btnSelect.EnableWindow(TRUE);
		printLog(_T("Video selection success"));
		break;
	case MSG_VIDEO_SELECTED_FAIL:
		m_btnSelect.EnableWindow(TRUE);
		printLog(_T("Video selection failed"));
		break;
	case MSG_NO_VIDEO:
		KillTimer(REQ_TIMEOUT_TIMER);
		AfxMessageBox(_T("There are no videos stored on the server."));
		break;
	case MSG_RECONNECT:
		if (mShowReconnectMsg == false)
		{
			clearVideoList();
			printLog(_T("The connection to the server has been lost."));
			//AfxMessageBox(_T("서버와의 연결이 끊어졌습니다."));
			mNetworkManager->resetStatus();
			resetStatus();
			mShowReconnectMsg = true;
		}
		break;
	default:
		break;		
	}
	return LRESULT();
}

int CMFCApplication1Dlg::getRadioBtnSecureMode()
{
	return m_radioBtnSecureMode;
}

int CMFCApplication1Dlg::getRadioBtnOperationMode()
{
	return m_radioBtnOperMode;
}

bool CMFCApplication1Dlg::getPlayStatus()
{
	return m_bModeStart;
}

BITMAPINFO* CMFCApplication1Dlg::getBitmapInfo()
{
	return m_pBitmapInfo;
}

NetworkManager* CMFCApplication1Dlg::getNetworkManager()
{
	return mNetworkManager;
}

bool CMFCApplication1Dlg::IsWorkingThread()
{
	return m_isWorkingThread;
}

Mat* CMFCApplication1Dlg::getMatImage()
{
	return &m_matImage;
}

void CMFCApplication1Dlg::clearVideoList()
{
	m_ListVideo.ResetContent();
}

void CMFCApplication1Dlg::setModeRadioBtnStatus()
{
	m_radioLearning.EnableWindow((mNetworkManager->isAdmin() && !m_bModeStart)? TRUE : FALSE); 
	m_radioRun.EnableWindow(!m_bModeStart);
	m_radioTestRun.EnableWindow(!m_bModeStart);
}


void CMFCApplication1Dlg::OnBnClickedButtonSelectVideo()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString itemSelected;
	int nSel = m_ListVideo.GetCurSel();
	if (nSel != LB_ERR)
	{
		m_btnSelect.EnableWindow(FALSE);
		CVideoSelectedIndexProtocol videoIdx(nSel);
		mNetworkManager->send_packet(videoIdx);
		SetTimer(REQ_TIMEOUT_TIMER, 5000, NULL);
	}
	else
		AfxMessageBox(_T("Please select a video to play."));
}


void CMFCApplication1Dlg::OnBnClickedButtonLogout()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	mShowReconnectMsg = false;
	m_isWorkingThread = false;
	resetStatus();
	mNetworkManager->closeTCPConnection();
	mNetworkManager->resetStatus();
	printLog(_T("Logout!"));
}

void CMFCApplication1Dlg::resetStatus()
{
	clearVideoList();
	m_radioSecure.EnableWindow(TRUE);
	m_radioNonSecure.EnableWindow(TRUE);
	m_EditID.EnableWindow(TRUE);
	m_EditPW.EnableWindow(TRUE);
	m_btnLogin.EnableWindow(TRUE);
	m_btnLogout.EnableWindow(FALSE);
	m_radioRun.EnableWindow(FALSE);
	m_radioLearning.EnableWindow(FALSE);
	m_radioTestRun.EnableWindow(FALSE);
	m_btnStart.EnableWindow(FALSE);
	m_EditName.EnableWindow(FALSE);
	m_EditImageNum.EnableWindow(FALSE);
	m_btnAdd.EnableWindow(FALSE);
	m_btnSelect.EnableWindow(FALSE);
	m_ListVideo.EnableWindow(FALSE);
	m_btnStart.SetWindowText(_T("Apply"));
	m_bModeStart = false;
	m_isWorkingThread = false;
	mLearningCnt = 0;
}
