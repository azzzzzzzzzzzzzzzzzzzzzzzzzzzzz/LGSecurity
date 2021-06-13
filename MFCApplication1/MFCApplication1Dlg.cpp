
// MFCApplication1Dlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "MFCApplication1.h"
#include "MFCApplication1Dlg.h"
#include "afxdialogex.h"
//#include "Common/NetworkTCP.h"
//#include "Common/TcpSendRecvJpeg.h"
#include <regex>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#define DEFAULT_PORT "55555"
#define DEFAULT_IP "192.168.0.223"
#define DRAW_TIMER 1000
#define ADD_TIMEOUT_TIMER 1001

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
TTcpConnectedPort* m_tcpConnectedPort = NULL;
struct sockaddr_in remaddr;	/* remote address */
socklen_t addrlen = sizeof(remaddr);/* length of addresses */

UINT ThreadForRecvImg(LPVOID param)
{
	CMFCApplication1Dlg* pMain = (CMFCApplication1Dlg*)param;

	// Send msg to server : start send image
	NetworkManager* networkManager = pMain->getNetworkManager();

	//networkManager->sendRequestImageStartToServer();

	while (pMain->IsWorkingThread())
	{
		Sleep(10);	
		
		// KSS
		int msgType = 0;
		long long timestamp = 0;
		string userId = "";
		int imgSize = 0;
		
		bool retvalue;
		retvalue = networkManager->readRecvImage(pMain->getMatImage(), msgType, timestamp, userId, imgSize);

		if (retvalue && pMain->getPlayStatus())
		{
			if (pMain->getBitmapInfo() == NULL) pMain->CreateBitmapInfo(pMain->getMatImage()->cols, pMain->getMatImage()->rows, pMain->getMatImage()->channels() * 8);
			pMain->DrawImage();
		}
	}

	return 0;
}

CMFCApplication1Dlg::CMFCApplication1Dlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MFCAPPLICATION1_DIALOG, pParent)
	, m_radioBtnSecureMode(0)
	, m_radioBtnOperMode(0)
	, m_bPlay(true)
	, m_bModeStart(false)
	, m_isWorkingThread(false)
	, m_pThread(NULL)
	, mNetworkManager(NULL)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCApplication1Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PICTURE, m_picture);
	DDX_Control(pDX, IDC_BUTTON_PLAY, m_btnPlay);
	DDX_Control(pDX, IDC_BUTTON_MODE_START, m_btnStart);
	DDX_Control(pDX, IDC_BUTTON_ADD_NEW_USER, m_btnAdd);
	DDX_Control(pDX, IDC_RADIO_MODE_LEARNING, m_radioLearning);
	DDX_Control(pDX, IDC_RADIO_MODE_RUN, m_radioRun);
	DDX_Control(pDX, IDC_RADIO_MODE_TEST_RUN, m_radioTestRun);
	DDX_Control(pDX, IDC_EDIT_NEW_USER_NAME, m_EditName);
	DDX_Control(pDX, IDC_EDIT_NEW_USER_IMGNUM, m_EditImageNum);
	DDX_Control(pDX, IDC_EDIT_INPUT_ID, m_EditID);
	DDX_Control(pDX, IDC_EDIT_INPUT_PW, m_EditPW);
	DDX_Control(pDX, IDC_EDIT_LOG, m_EditLog);
	DDX_Radio(pDX, IDC_RADIO_SECURE, (int&)m_radioBtnSecureMode);
	DDX_Radio(pDX, IDC_RADIO_MODE_LEARNING, (int&)m_radioBtnOperMode);
	DDX_Control(pDX, IDC_SPIN_IMAGE_NUM, m_spinIMGNum);
	DDX_Control(pDX, IDC_LIST_VIDEO, m_ListVideo);
	DDX_Control(pDX, IDC_BUTTON_SELECT_VIDEO, m_btnSelect);
}

BEGIN_MESSAGE_MAP(CMFCApplication1Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_PLAY, &CMFCApplication1Dlg::OnBnClickedButtonPlay)
	ON_BN_CLICKED(IDC_BUTTON_LOGIN, &CMFCApplication1Dlg::OnBnClickedButtonLogin)
	ON_BN_CLICKED(IDC_BUTTON_MODE_START, &CMFCApplication1Dlg::OnBnClickedButtonModeStart)
	ON_BN_CLICKED(IDC_BUTTON_ADD_NEW_USER, &CMFCApplication1Dlg::OnBnClickedButtonAddNewUser)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_RADIO_SECURE, IDC_RADIO_NON_SECURE, &CMFCApplication1Dlg::OnBnClickSecureRadioCtrl)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_RADIO_MODE_LEARNING, IDC_RADIO_MODE_TEST_RUN, &CMFCApplication1Dlg::OnBnClickOperModeRadioCtrl)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_IMAGE_NUM, &CMFCApplication1Dlg::OnDeltaposSpinImageNum)
	ON_MESSAGE(MESSAGE_SHOW_POPUPDLG, &CMFCApplication1Dlg::showPopupDialog)
	ON_MESSAGE(MESSAGE_ADD_ITEM_TO_LIST, &CMFCApplication1Dlg::addVideoItemToList)
	ON_BN_CLICKED(IDC_BUTTON_SELECT_VIDEO, &CMFCApplication1Dlg::OnBnClickedButtonSelectVideo)
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
	m_EditImageNum.SetWindowText(_T("1"));
	m_spinIMGNum.SetRange(1, 5);
	m_spinIMGNum.SetPos(1);
	m_radioLearning.SetCheck(FALSE);
	m_radioRun.SetCheck(TRUE);
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
	//CloseTcpConnectedPort(&m_tcpConnectedPort);
	delete mNetworkManager;
	CDialogEx::OnClose();
}


void CMFCApplication1Dlg::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent)
	{
	case DRAW_TIMER:

		break;
	case ADD_TIMEOUT_TIMER:
		m_EditName.EnableWindow(true);
		m_EditImageNum.EnableWindow(true);
		m_btnAdd.EnableWindow(true);
		KillTimer(ADD_TIMEOUT_TIMER);
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
	log.Format(_T("[%s]	%s  \r\n"), (LPCTSTR)time.Format(L"%Y-%m-%d %H:%M:%S"), logStr);
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


void CMFCApplication1Dlg::OnBnClickedButtonPlay()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_bPlay = !m_bPlay;
	m_btnPlay.SetWindowText((m_bPlay)? CString("||") : CString("▶"));
}


void CMFCApplication1Dlg::OnBnClickedButtonLogin()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	mNetworkManager->setSecureMode((m_radioBtnSecureMode == MODE_SECURE) ? true : false);
	CString id;
	CString pw;
	m_EditID.GetWindowTextW(id);
	m_EditPW.GetWindowTextW(pw);
	if (id.IsEmpty() || checkIDPW(id) == false || pw.IsEmpty() || checkIDPW(pw) == false)
	{
		AfxMessageBox(_T("유효한 ID, PW를 입력해주세요. (알파벳, 숫자만 허용)"));
		return;
	}

	// KSS TODO
	if (false == mNetworkManager->openTcpConnection())
	{
		printf(" Fail OpenTcpConnection\n");
		return;
	}

	// send login ID, PW
	//mNetworkManager->sendRequestLoginToServer(string(CT2CA(id)), string(CT2CA(pw)));

	// receive login result

	// login success
	
	m_isWorkingThread = true;
	m_pThread = AfxBeginThread(ThreadForRecvImg, this);

	m_btnStart.EnableWindow(TRUE);
	setModeRadioBtnStatus();
}

void CMFCApplication1Dlg::OnBnClickedButtonModeStart()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_bModeStart = !m_bModeStart;
	m_btnStart.SetWindowText((m_bModeStart) ? CString("Stop") : CString("Start"));

	setModeRadioBtnStatus();

	if (m_bModeStart)//start
	{
		if (m_radioBtnOperMode == MODE_LEARNING)
		{
			m_EditName.EnableWindow(TRUE);
			m_EditImageNum.EnableWindow(TRUE);
			m_btnAdd.EnableWindow(TRUE);
		}
		else if (m_radioBtnOperMode == MODE_TEST_RUN)
		{
			m_ListVideo.EnableWindow(TRUE);
			m_btnSelect.EnableWindow(TRUE);
			//send video request message to server
		}
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
	CString name;
	CString num;
	m_EditName.GetWindowTextW(name);
	m_EditImageNum.GetWindowTextW(num);
	int cnt = _ttoi(num);
	if (name.IsEmpty() || checkString(name) == false)
	{
		AfxMessageBox(_T("유효한 이름을 입력해주세요. (알파벳만 허용)"));
		return;
	}

	// TODO: Add request
	printLog(_T("add button"));
	//msgTest(this->m_hWnd);
	m_EditName.EnableWindow(false);
	m_EditImageNum.EnableWindow(false);
	m_btnAdd.EnableWindow(false);
	SetTimer(ADD_TIMEOUT_TIMER, 3000, NULL);
}

void CMFCApplication1Dlg::OnBnClickSecureRadioCtrl(UINT ID)
{
	UpdateData(true);
	switch (ID)
	{
	case IDC_RADIO_SECURE:
		//AfxMessageBox(_T("IDC_RADIO_SECURE"));
		break;
	case IDC_RADIO_NON_SECURE:
		//AfxMessageBox(_T("IDC_RADIO_NON_SECURE"));
		break;
	default:
		break;
	}
	printf("mode1 %d\n", m_radioBtnSecureMode);
}

void CMFCApplication1Dlg::OnBnClickOperModeRadioCtrl(UINT ID)
{
	UpdateData(true);
	switch (ID)
	{
	case IDC_RADIO_MODE_LEARNING:
		//AfxMessageBox(_T("IDC_RADIO_MODE_LEARNING"));
		break;
	case IDC_RADIO_MODE_RUN:
		//AfxMessageBox(_T("IDC_RADIO_MODE_RUN"));
		break;
	case IDC_RADIO_MODE_TEST_RUN:
		//AfxMessageBox(_T("IDC_RADIO_TEST_RUN"));
		break;
	default:
		break;
	}
	printf("mode2 %d\n", m_radioBtnOperMode);
}

void CMFCApplication1Dlg::OnDeltaposSpinImageNum(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	int val = pNMUpDown->iPos + pNMUpDown->iDelta;

	if ((1 <= val) && (val <= 5))
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

LRESULT CMFCApplication1Dlg::showPopupDialog(WPARAM wParam, LPARAM IParam)
{
	switch (wParam)
	{
	case MSG_SERVER_ERROR:
		AfxMessageBox(_T("서버에 에러가 발생하였습니다."));
		break;
	case MSG_LEARNING_COMPLETE:
		AfxMessageBox(_T("이름 추가가 완료되었습니다."));
		break;
	case MSG_LOGIN_DUPLICATED_ID:
		AfxMessageBox(_T("중복된 ID입니다."));
		break;
	case MSG_LOGIN_ERROR:
		AfxMessageBox(_T("로그인에 실패하였습니다."));
		break;
	case MSG_NO_PERMISSION:
		AfxMessageBox(_T("권한이 없습니다."));
		break;
	case MSG_OK:
		AfxMessageBox(_T("성공!"));
		break;
	case MSG_NO_VIDEO:
		AfxMessageBox(_T("서버에 저장된 비디오가 없습니다."));
		break;
	default:
		AfxMessageBox(_T("정의되지 않은 메세지입니다."));
		break;
	}
	return LRESULT();
}

LRESULT CMFCApplication1Dlg::addVideoItemToList(WPARAM wParam, LPARAM IParam)
{
	CString* pstrString = (CString*)IParam;
	CString strString = pstrString->GetString();
	switch (wParam)
	{
	case MESSAGE_ADD_ITEM_TO_LIST:
		m_ListVideo.AddString(strString);
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
	return m_bPlay;
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
		m_ListVideo.GetText(nSel, itemSelected);
		//send selected video name to server

	}
	else
		AfxMessageBox(_T("비디오를 선택해주세요."));
}
