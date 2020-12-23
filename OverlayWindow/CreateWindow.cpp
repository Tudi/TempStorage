#include <stdio.h>
#include <windows.h>
#include <list>
#include "PointsToRender.h"

#ifndef MAX
	#define MAX(a,b) (((a)>(b))?(a):(b))
#endif
#ifndef MIN
	#define MIN(a,b) (((a)<(b))?(a):(b))
#endif

HWND hwnd;
class PixelsToClear
{
	public:
	PixelsToClear(int a, int b)
	{
		x = a;
		y = b;
	}
	int x,y;
};
int WindowCreateWidth, WindowCreateHeight;
int WindowCreateStartX, WindowCreateStartY;
std::list<PixelsToClear*> OldPixels;
CRITICAL_SECTION OnePaint;
#define MAX_POINT_COUNT 10000

static int imagecounter=0;
static int RedrawPending=0;
__int64 StartTickCount = 0;
IconPositionsToRender *PointsToRender2;
int PointsToRenderCount2;

void PaintWindow( HWND hWnd )
{
	EnterCriticalSection(&OnePaint);
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hWnd, &ps);
	RECT rect;
	GetClientRect(hWnd, &rect);
	int width=rect.right;
	int height=rect.bottom;

	HDC backbuffDC = CreateCompatibleDC(hdc);

	HBITMAP backbuffer = CreateCompatibleBitmap( hdc, width, height);

	int savedDC = SaveDC(backbuffDC);
	SelectObject( backbuffDC, backbuffer );
	HBRUSH hBrush = CreateSolidBrush(RGB(255,255,255));
	FillRect(backbuffDC,&rect,hBrush);
	DeleteObject(hBrush);
	
//	printf("got repaint msg %d, pixels to paint %d %p\n", imagecounter++, (int)PointsToRender.size(), hwnd);
	for( int itr = 0; itr <PointsToRenderCount2; itr++)
	{
		float Smile = PointsToRender2[itr].SmilePCT;
		Smile = MAX(Smile * 10 - 9,0);
		float Sad = MAX(0,(PointsToRender2[itr].SadPCT * 10 - 10)*5);
		unsigned char R = MAX(0,MIN(255 * Sad, 255));
		unsigned char G = MAX(0,MIN(255 * Smile, 255));
//		int bx = static_cast<int>(round(PointsToRender2[itr].x * WindowCreateWidth));
//		int by = static_cast<int>(round(PointsToRender2[itr].y * WindowCreateHeight));
		int bx = (static_cast<int>(round(PointsToRender2[itr].x * WindowCreateWidth)) - 20);
		int by = static_cast<int>(round(PointsToRender2[itr].y * WindowCreateHeight));
//		printf("\t %f %f = %d %d\n", PointsToRender[itr].x,PointsToRender[itr].y, bx, by);
//		printf("\t %d %d %f %f %d %d\n", bx, by, Smile, Sad, R, G);
		SetPixel( backbuffDC, bx, by, RGB(R,G,0) );
#if 1		
		for(int i=0;i<15;i++)
			for(int j=0;j<15;j++)
			{
				int tx = bx +i;
				int ty = by +j;
				if(tx>0&&tx<WindowCreateWidth && ty>0 && ty < WindowCreateHeight)
					SetPixel( backbuffDC, tx, ty, RGB(R,G,0) ); 
			}
#endif			
	}
	if(StartTickCount==0)
		StartTickCount = GetTickCount()-1;
	__int64 TimePassed = GetTickCount() - StartTickCount;
	float FPS = (float)imagecounter / ((float)TimePassed/1000);
//	printf("got repaint msg %d, pixels to paint %d FPS %f\n", imagecounter++, (int)PointsToRenderCount2, FPS);
	LeaveCriticalSection(&OnePaint);
	
	BitBlt(hdc,0,0,width,height,backbuffDC,0,0,SRCCOPY);
	RestoreDC(backbuffDC,savedDC);

	DeleteObject(backbuffer);
	DeleteDC(backbuffDC);

	EndPaint(hWnd, &ps);
			
	RedrawPending = 0;
}

void PaintWindow2( HWND hwnd )
{
	EnterCriticalSection(&OnePaint);
  PAINTSTRUCT ps;
  HDC hdc = BeginPaint( hwnd, &ps );
	for(auto itr = OldPixels.begin(); itr != OldPixels.end(); itr++)
	{
		SetPixel( hdc, (*itr)->x, (*itr)->y, RGB(255,255,255) ); 
		delete *itr;
	}
	OldPixels.clear();
	
	LockPOIList();
//	printf("got repaint msg %d, pixels to paint %d %p\n", imagecounter++, (int)PointsToRender.size(), hwnd);
	for( int itr = 0; itr <PointsToRenderCount; itr++)
	{
		int bx = static_cast<int>(round(PointsToRender[itr].x * WindowCreateWidth));
		int by = static_cast<int>(round(PointsToRender[itr].y * WindowCreateHeight));
//		printf("\t %f %f = %d %d\n", PointsToRender[itr].x,PointsToRender[itr].y, bx, by);
		SetPixel( hdc, bx, by, RGB(255,0,0) );
#if 0		
		for(int i=0;i<5;i++)
			for(int j=0;j<5;j++)
			{
				int tx = bx +i;
				int ty = by +j;
				if(tx>0&&tx<WindowCreateWidth && ty>0 && ty < WindowCreateHeight)
					SetPixel( hdc, tx, ty, RGB(255,0,0) ); 
			}
#endif			
		OldPixels.push_front(new PixelsToClear(bx,by));
	}
	printf("got repaint msg %d, pixels to paint %d %p\n", imagecounter++, (int)PointsToRenderCount, hwnd);
	UnlockPOIList();
    EndPaint( hwnd, &ps );
	LeaveCriticalSection(&OnePaint);
	RedrawPending = 0;
}

void PaintFrom(int Width, int Height, int Stride, const unsigned char *data)
{
	if(RedrawPending==1)
		return;
	RedrawPending = 1;
	EnterCriticalSection(&OnePaint);
	PointsToRenderCount2 = PointsToRenderCount;
	if(MAX_POINT_COUNT<PointsToRenderCount2)
		PointsToRenderCount2 = MAX_POINT_COUNT;
	memcpy(PointsToRender2,PointsToRender,PointsToRenderCount2 * sizeof(IconPositionsToRender));
//	printf("New paint %d w %d, h %d, s %d\n",imagecounter++, Width, Height, Stride);
//  PAINTSTRUCT ps;
//  HDC hdc = BeginPaint( hwnd, &ps );
//	for(auto itr = NewPixels.begin(); itr != NewPixels.end(); itr++)
//		delete *itr;
/*	NewPixels.clear();
  for(int y=0;y<Height;y++)
	for(int x=0;x<Width;x++)
	{
		if(data[y * Stride + x * 3 + 0] != 255
			|| data[y * Stride + x * 3 + 1] != 255
			|| data[y * Stride + x * 3 + 2] != 255 )
			{
//			SetPixel( hdc, x, y, RGB( data[y * Stride + x * 3 + 0], data[y * Stride + x * 3 + 1], data[y * Stride + x * 3 + 2] ) ); 
//			SetPixel( hdc, x, y, RGB( 255, 0, 0 ) ); 
//			printf("Found a pixel at %d %d\n", x, y);
			NewPixels.push_front(new PixelsToSet(x,y,RGB( data[y * Stride + x * 3 + 0], data[y * Stride + x * 3 + 1], data[y * Stride + x * 3 + 2] )));
			}
//		else
//			SetPixel( hdc, x, y, RGB( 255, 255, 255 ) );  // transparent
	}*/
//  EndPaint( hwnd, &ps );
//RECT lpRect{0,0,640,480};
//InvalidateRect(hwnd,&lpRect,true);
//UpdateWindow(hwnd);
RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE);
//  MoveWindow(hwnd, 200 + 640 + (imagecounter & 1), 200, 640, 480, true );
//  PaintWindow(hwnd);
//or else we delete the window too fast
//while(RedrawPending==1)
//Sleep(1);
LeaveCriticalSection(&OnePaint);
}
/*
void ClearBackground(HWND hwnd)
{
    RECT rcWin;
    RECT rcWnd;
//    HWND parWnd = GetParent( hwnd ); // Get the parent window.
    HWND parWnd = ( hwnd ); // Get the parent window.
    HDC parDc = GetDC( hwnd ); // Get its DC.

    GetWindowRect( hwnd, &rcWnd );
    ScreenToClient( parWnd, &rcWnd ); // Convert to the parent's co-ordinates

    GetClipBox(hdc, &rcWin );
    // Copy from parent DC.
    BitBlt( hdc, rcWin.left, rcWin.top, rcWin.right - rcWin.left,
        rcWin.bottom - rcWin.top, parDC, rcWnd.left, rcWnd.top, SRC_COPY );

    ReleaseDC( parWnd, parDC );
}*/

LRESULT CALLBACK WndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
  switch (msg)
  {
//    case WM_KEYDOWN: // same as pressing the X button:
//    case WM_CLOSE:   DestroyWindow( hwnd ); return 0;
    case WM_DESTROY: PostQuitMessage( 0 );  return 0;
    case WM_PAINT:   PaintWindow( hwnd );   return 0;
//	case WM_ERASEBKGND: ClearBackground(hwnd); return 0;
	case WM_ERASEBKGND:    return 1;
  }
  return DefWindowProc( hwnd, msg, wParam, lParam );
}

void StayOnTop()
{	
 RECT rect;
 
 // get the current window size and position
 GetWindowRect( hwnd, &rect );
 
 // now change the size, position, and Z order 
 // of the window.
 ::SetWindowPos(hwnd ,       // handle to window
                HWND_TOPMOST,  // placement-order handle
                rect.left,     // horizontal position
                rect.top,      // vertical position
                rect.right - rect.left,  // width
                rect.bottom - rect.top, // height
                SWP_SHOWWINDOW // window-positioning options
				);
}

DWORD ScreenCap_CreateWindow_(LPVOID arg)
{
	InitializeCriticalSection(&OnePaint);

  // Register window class
//  HBRUSH br = GetStockObject( BLACK_BRUSH );
  WNDCLASSA wc =
  {
    0, WndProc, 0, 0, 0, 		//style,lpfnWndProc,cbClsExtra,cbWndExtra,hInstance
    LoadIcon( NULL, IDI_APPLICATION ),
    LoadCursor( NULL, IDC_ARROW ),
    NULL, // background color == black
    NULL, // no menu
    "ExampleWindowClass"
  };
  
  ATOM wClass = RegisterClassA( &wc );
  if (!wClass)
  {
    fprintf( stderr, "%s\n", "Couldn’t create Window Class" );
    return 1;
  }
  
  // Create the window
  hwnd = CreateWindowA(
    MAKEINTATOM( wClass ),
    "transparent window",     // window title
    WS_OVERLAPPEDWINDOW, // title bar, thick borders, etc.
    CW_USEDEFAULT, CW_USEDEFAULT, WindowCreateWidth, WindowCreateHeight,
    NULL, // no parent window
    NULL, // no menu
    GetModuleHandle( NULL ),  // EXE's HINSTANCE
    NULL  // no magic user data
  );
  if (!hwnd)
  {
    fprintf( stderr, "%ld\n", GetLastError() );
    fprintf( stderr, "%s\n", "Failed to create Window" );
    return 1;
  }
//  MoveWindow(hwnd, 200 + 640, 200 - 100, 640, 480, false );
  MoveWindow(hwnd, WindowCreateStartX-5, WindowCreateStartY-35, WindowCreateWidth, WindowCreateHeight, false );
//  MoveWindow(hwnd, 100, 200 - 150, 640, 480, false );
  
  SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
  SetLayeredWindowAttributes(hwnd, RGB(255,255,255), 0, LWA_COLORKEY); // make white pixels transparent
  
  // Make window visible
  ShowWindow( hwnd, SW_SHOWNORMAL );
  StayOnTop();
  
  // Event loop
  MSG msg;
  while (GetMessage( &msg, NULL, 0, 0 ) > 0)
  {
    TranslateMessage( &msg );
    DispatchMessage( &msg );
  }
  return msg.wParam;
}

DWORD ScreenCap_MonitorWindow_(LPVOID arg)
{
	while(!hwnd)
		Sleep(100);
	while(hwnd)
	{
		HWND MonitoredWnd = GetForegroundWindow();
		if(MonitoredWnd != hwnd)
		{
			RECT rect1;
			GetWindowRect( MonitoredWnd, &rect1 );
			RECT rect2;
			GetWindowRect( hwnd, &rect2 );
			int Width1 = rect1.right - rect1.left;
			int Height1 = rect1.bottom- rect1.top;
			int Width2 = rect2.right - rect2.left;
			int Height2 = rect2.bottom - rect2.top;
			if(rect1.top-35 != rect2.top || rect1.left-5 != rect2.left || Width1+5!=Width2 || Height1 +35!= Height2)
			{
				printf("moving to : %d %d %d %d, from %d %d %d %d\n",rect1.left-5, rect1.top-35, Width1+5, Height1+35, rect2.left, rect2.top, Width2, Height2);
				WindowCreateWidth = Width1;
				WindowCreateHeight = Height1;
				MoveWindow(hwnd, rect1.left-5, rect1.top-35, Width1+5, Height1+35, false );
			}
		}
		Sleep(500);
	}
	return 0;
}

int ScreenCap_CreateWindow(int Startx, int StartY, int Widht, int Height)
{
	WindowCreateStartX = Startx;
	WindowCreateStartY = StartY;
	WindowCreateWidth = Widht;
	WindowCreateHeight = Height;
	PointsToRender2 = (IconPositionsToRender*)malloc(sizeof(IconPositionsToRender)*MAX_POINT_COUNT);
	PointsToRenderCount2 = 0;

    HANDLE thread1 = CreateThread(NULL, 1, (LPTHREAD_START_ROUTINE)ScreenCap_CreateWindow_, (LPVOID)NULL, 0, NULL);
    HANDLE thread2 = CreateThread(NULL, 1, (LPTHREAD_START_ROUTINE)ScreenCap_MonitorWindow_, (LPVOID)NULL, 0, NULL);
    if (thread1 == NULL || thread2 == NULL)
    {
        return 1;
    }
    CloseHandle(thread1);
    CloseHandle(thread2);
	return 0;
}