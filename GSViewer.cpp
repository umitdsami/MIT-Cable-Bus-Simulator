// GSViewer.cpp 
// Sample and test code for GSView and GSMesh modules

#include "stdafx.h"
#include "GSViewer.h"
#include "LpsTktGSView.h"
#include "LogStream_1.h"
#include "DialogDataWrapper.h"
#include <ctime>
#include <fstream>

using namespace LpsTktGSV; //you need this namespace for creating Lps::CharString

// Logger for Toolkit messages
LogStream_1 logger("GSViewTest.log");

// Example instance of GSView interface
GSView gsViewer;
DialogDataWrapper dialogDataWrapperM;

// Text buffer to pass text message to dialog box.
static Lps::CharString		GSViewer_TextMessage;

#define MAX_LOADSTRING 100

// Test integrity of heap
bool heapCheck() {
	int heapstatus = _heapchk();
   switch( heapstatus )
   {
   case _HEAPOK:
		gsViewer.log() << "Heap Check reports no errors"; gsViewer.log().dsplInfo();
		return true;
      break;
   case _HEAPEMPTY:
		gsViewer.log() << "Heap Check reports no errors"; gsViewer.log().dsplInfo();
		return true;
      break;
   case _HEAPBADBEGIN:
		gsViewer.log() << "Heap Check reports bad start of heap"; gsViewer.log().dsplErr();
      break;
   case _HEAPBADNODE:
		gsViewer.log() << "Heap Check reports bad node in heap"; gsViewer.log().dsplErr();
      break;
   }
	return false;
}

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	FileDialog(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	TextMessage(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_TESTGSVIEW_WIN32, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_TESTGSVIEW_WIN32);

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}

//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	// NOTE 9.6.05 WAL
	// CS_DBLCLKS is needed for windows to provide that type of message
	wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_TESTGSVIEW_WIN32);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCTSTR)IDC_TESTGSVIEW_WIN32;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HANDLE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//        The GSView instance is set up.
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;

	hInst = hInstance; // Store instance handle in our global variable

	// cheap shot static way to link to app.
	dialogDataWrapperM.hInstM = hInst;
	dialogDataWrapperM.pMessageStreamM = &logger;
	dialogDataWrapperM.pGSViewerM = &gsViewer;

   // NOTE 9.6.05 WAL add WS_CLIPSIBLINGS | WS_CLIPCHILDREN for OpenGL
	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		return FALSE;
	}

	// -------------------------------------- Link Log Stream
	gsViewer.setMsgStream( &logger );
	gsViewer.log() << "LogStream linked to Message Stream"; 
	
	gsViewer.dsplInfo();

	// ------ Set up the Example window and create OpenGL context
	gsViewer.windowAdd(hWnd);
	gsViewer.sceneUseTopLevelDisplayList(true);

	// ------ Put constant tools on the tool queue
	gsViewer.toolAdd(GSView::ROTATEVIEW, GSView::RUNALWAYS);
	gsViewer.toolAdd(GSView::ZOOMVIEW, GSView::RUNALWAYS);
	gsViewer.toolAdd(GSView::PANVIEW, GSView::RUNALWAYS);
	//gsViewer.toolAdd(GSView::AUTOPICK,GSView::RUNALWAYS);
	//gsViewer.toolAdd(GSView::EDITCOMPONENT,GSView::RUNALWAYS);

	// -----Load colors
	gsViewer.colorAddW3CX11Colors();

	gsViewer.sceneDisplaySetShadeModel(hWnd,'S',true);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	heapCheck();

	return TRUE;
}

// This is the main Windows loop.  This function processes messages
// as they are sent to the window linked to this application.
// Since this is an example and test platform, we dont bother to
// pull out subroutines, however in normal practice an application
// programmer would want to use classes and their methods to provide
// the functionality for most message responses.
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;

	switch (message) 
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam); 
		wmEvent = HIWORD(wParam); 
		
		switch (wmId) {
			case IDM_ABOUT:
				DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
			break;

			case IDM_EXIT:
			{
				dialogDataWrapperM.closeDatabase1();
				dialogDataWrapperM.closeDatabase2();
				DestroyWindow(hWnd);
			}
			break;

			case ID_OPENALEAPSDATABASE_DATABASE1:  
			{	  
				if (!dialogDataWrapperM.getAndOpenLeapsDatabase1())
				{
					GSViewer_TextMessage = "COULD NOT OPEN LEAPS DATABASE!";
					DialogBox(hInst, (LPCTSTR)IDD_TEXTMESSAGE, hWnd, (DLGPROC)TextMessage);
				}
				if (!dialogDataWrapperM.selectStudy1())
				{
					GSViewer_TextMessage = "COULD NOT OPEN STUDY!";
					DialogBox(hInst, (LPCTSTR)IDD_TEXTMESSAGE, hWnd, (DLGPROC)TextMessage);
				}
				if (!dialogDataWrapperM.selectConcept1())
				{
					GSViewer_TextMessage = "COULD NOT OPEN CONCEPT!";
					DialogBox(hInst, (LPCTSTR)IDD_TEXTMESSAGE, hWnd, (DLGPROC)TextMessage);
				}
				gsViewer.log() << "Successfully opened Database 1 -> Study 1 -> Concept 1"; gsViewer.log().dsplInfo();


				//calculate number of electric zones
				int ShipNum = 1;
				dialogDataWrapperM.GetNumOfElectZones(&ShipNum);
				gsViewer.log() << "Number of Electric Zones in Ship " << ShipNum << " is : " << dialogDataWrapperM.mymap1.size();
				gsViewer.dsplInfo();

				//calcuate geometric boundary of each electric zone
				//#define CLOCKING_TEST  //uncomment this line to deactivate clocking test
				//#ifndef CLOCKING_TEST
				//#define CLOCKING_TEST
				//clock_t start;
				//clock_t finish;
				//std::ofstream fout ("CLOCKING_TEST.txt", std::ofstream::out);
				//start = clock();
				dialogDataWrapperM.GetBoundaryOfElectricZones(&ShipNum);
				//finish = clock() - start;
				//double interval = finish / (double)CLOCKS_PER_SEC;
				//fout << "Clock cycles is : " << finish << " and seconds elapsed is :" << interval;
				//#endif 

				auto ii = dialogDataWrapperM.ezoneNames.begin();  // always use auto for std::vector<Lps::CharString>::iterator

				if (dialogDataWrapperM.ElectZoneBoundary.empty())
				{
					gsViewer.log() << "There are NO faces affiliated with Electric Zones in this ship. Boundary is EMPTY...Exiting! ";
					gsViewer.log().dsplInfo();
					break;
				}
				for (int id = 0; id < dialogDataWrapperM.ElectZoneBoundary.size(); id++)
				{
					if (id % 6 == 0) { gsViewer.log().dsplInfo(); }
					gsViewer.log() << " Value in cell [ " << id << " ] is : " << dialogDataWrapperM.ElectZoneBoundary[id];
					gsViewer.dsplInfo();
				}
				for (int idx = 0; idx < dialogDataWrapperM.xsort.size(); idx++)
				{
					gsViewer.log() << " X axis cell [ " << idx << " ] value is : " << dialogDataWrapperM.xsort[idx];
					gsViewer.log().dsplInfo();
				}
				for (int idy = 0; idy < dialogDataWrapperM.ysort.size(); idy++)
				{
					gsViewer.log() << " Y axis cell [ " << idy << " ] value is : " << dialogDataWrapperM.ysort[idy];
					gsViewer.log().dsplInfo();
				}
				for (int idz = 0; idz < dialogDataWrapperM.zsort.size(); idz++)
				{
					gsViewer.log() << " Z axis cell [ " << idz << " ] value is : " << dialogDataWrapperM.zsort[idz];
					gsViewer.log().dsplInfo();
				}		
			}
			break;

			case ID_CLOSELEAPSDATABASE_DATABASE1:
			{
				  dialogDataWrapperM.closeDatabase1();
			}
			break;
			
			case ID_OPENALEAPSDATABASE_DATABASE2:
			{
				int buscounter = 0;
				if (!dialogDataWrapperM.getAndOpenLeapsDatabase2())
				{
					GSViewer_TextMessage = "COULD NOT OPEN LEAPS DATABASE!";
					DialogBox(hInst, (LPCTSTR)IDD_TEXTMESSAGE, hWnd, (DLGPROC)TextMessage);
				}
				if (!dialogDataWrapperM.selectStudy2())
				{
					GSViewer_TextMessage = "COULD NOT OPEN STUDY!";
					DialogBox(hInst, (LPCTSTR)IDD_TEXTMESSAGE, hWnd, (DLGPROC)TextMessage);
				}
				for (bussecnum = 0; bussecnum < total_bus; bussecnum++)  //create and associate all leaps objects in target db
				{
					if (!dialogDataWrapperM.selectConcept2())
					{
						GSViewer_TextMessage = "COULD NOT OPEN CONCEPT!";
						DialogBox(hInst, (LPCTSTR)IDD_TEXTMESSAGE, hWnd, (DLGPROC)TextMessage);
					}
					dialogDataWrapperM.Function0();
				}
				
				dialogDataWrapperM.BuildCompVector();
				

				for (linknum = 0; linknum < total_bus; linknum++) //link all leaps objects in target db
				{
					//GSViewer_TextMessage = dialogDataWrapperM.linkTwoComponents();
				}
				
				//dialogDataWrapperM.AddCompExContoRootSysCon(); //connect all CECs' from source db to root SC in target db
			}
			break;

			case ID_CLOSELEAPSDATABASE_DATABASE2:
			{			
				dialogDataWrapperM.closeDatabase2();
			}
			break;

			default:
				return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

	case WM_PAINT:
	{
    	 gsViewer.windowOnDraw(hWnd);
	}
		break;

	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_RBUTTONDBLCLK:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MBUTTONDBLCLK:
		// All of the above are handled
		gsViewer.windowOnMouseOperation(hWnd,message,wParam,lParam);
		break;

	case WM_KEYDOWN:
	case WM_CHAR:
		// All of the above are handled
		gsViewer.windowOnKeyOperation(hWnd,message,wParam,lParam);
		break;

	case WM_SIZE:
		// OnSize command
		gsViewer.windowOnSize(hWnd);
		break;

	case WM_ERASEBKGND:
		// Note that this function does nothing -
		// it is just a reminder that DefWindowProc()
		// must not be called.
		gsViewer.windowOnEraseBackground();
		break;

	case WM_DESTROY:
		// this occurs when red x button is hit

		gsViewer.windowClear(hWnd);
		// Clear prior to destructor call in order to trace
		gsViewer.Clear();

		heapCheck();

		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}

// Message handler for Text Message dialog.
LRESULT CALLBACK TextMessage(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		SetDlgItemText(hDlg, IDC_TEXTMESSAGE, GSViewer_TextMessage.c_str());
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}

