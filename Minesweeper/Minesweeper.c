#include <stdbool.h>
#include "..\DLL\DLL.h"
#include <process.h>//多线程为了相对精确的计时。。。
#include "resource.h"

void timer(void* Parameter);
void erasetext(HWND hwnd,HDC hdc);
LRESULT __stdcall main(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);
LRESULT __stdcall WndProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);

int __stdcall _tWinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPTSTR lpCmdLine,int nCmdShow)
{
	WNDCLASS wndclass;
	wndclass.hbrBackground=CreateSolidBrush(RGB(215,225,235));
	wndclass.hCursor=LoadCursor(0,IDC_ARROW);
	wndclass.hIcon=LoadIcon(hInstance,(TCHAR*)IDI_ICON1);
	wndclass.hInstance=hInstance;
	wndclass.lpfnWndProc=WndProc;
	wndclass.lpszClassName=_T("Minesweeper");
	wndclass.lpszMenuName=0;
	wndclass.style=CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
	wndclass.cbWndExtra=0;
	wndclass.cbClsExtra=0;
	RegisterClass(&wndclass);
	HWND hwnd=CreateWindow(_T("Minesweeper"),_T("Minesweeper"),WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_MAXIMIZEBOX,
		CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,0,0,hInstance,0);
	ShowWindow(hwnd,SW_MAXIMIZE);
	MSG msg;
	while(GetMessage(&msg,0,0,0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.message;
}

LRESULT __stdcall WndProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	static HWND hwndmain;static RECT rect;
	switch(message)
	{
	case WM_CREATE:
	{
		HINSTANCE hInstance=((CREATESTRUCT*)lParam)->hInstance;
		WNDCLASS wndclass={0};
		wndclass.hbrBackground=GetStockObject(WHITE_BRUSH);
		wndclass.hCursor=LoadCursor(0,IDC_ARROW);
		wndclass.hIcon=LoadIcon(0,IDI_APPLICATION);
		wndclass.hInstance=hInstance;
		wndclass.lpfnWndProc=main;
		wndclass.lpszClassName=_T("main");
		wndclass.style=CS_HREDRAW|CS_VREDRAW;
		RegisterClass(&wndclass);
		hwndmain=CreateWindow(_T("main"),_T(""),WS_CHILD|WS_VISIBLE,0,0,0,0,hwnd,0,hInstance,0);
		HDC hdc=GetDC(hwnd);
		SetBkColor(hdc,RGB(215,225,235));
		LOGFONT logfont={0};
		logfont.lfHeight=18;
		_tcscpy_s(logfont.lfFaceName,_tcslen(_T("宋体"))+1,_T("宋体"));
		HFONT hfont=CreateFontIndirect(&logfont);
		DeleteObject(SelectObject(hdc,hfont));
		ReleaseDC(hwnd,hdc);
		return 0;
	}
	case WM_SIZE:
	{
		rect.right=LOWORD(lParam);
		rect.bottom=HIWORD(lParam);
		int size;
		if(rect.right>rect.bottom)
			size=(int)(0.9*rect.bottom/16);
		else
			size=rect.right/30;
		MoveWindow(hwndmain,(rect.right-size*30)/2,(rect.bottom-size*16)/2,size*30,size*16,1);
		return 0;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd,message,wParam,lParam);
}

void initializeunknown(int* block)
{
	for (size_t i = 0; i < 480; i++)
	{
		block[i] = 0x80000000;
	}
}

LRESULT __stdcall main(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	static HBITMAP* hbitmap;
	static HANDLE hevent,hthread;
	static int *block,size,time,mine=99;
	static bool first_click;
	switch(message)
	{
	case WM_CREATE:
	{
		block=malloc(480*sizeof(int));
		first_click = true;
		initializeunknown(block);
		hbitmap=loadbitmap();
		hevent=CreateEvent(0,1,0,_T("continue"));
		return 0;
	}
	case WM_SIZE:
	{
		int cx=LOWORD(lParam);
		size=cx/30;
		return 0;
	}
	case WM_TIMER:
	{
		if(time<999)
			time++;
		HWND hwndparent=GetParent(hwnd);
		HDC hdc=GetDC(hwndparent);
		erasetext(hwnd,hdc);
		TCHAR* string=malloc(64*sizeof(TCHAR));
		TextOut(hdc,0,0,string,_stprintf_s(string,64,_T("已用时间:%ds     剩余地雷数:%d"),time,mine));
		free(string);
		ReleaseDC(hwndparent,hdc);
		return 0;
	}
	case WM_LBUTTONDOWN:
	{
		int i=LOWORD(lParam)/size,j=HIWORD(lParam)/size,k=j*30+i;
		if(!hthread)
		{
			SetEvent(hevent);
			HWND* hwndcopy=malloc(sizeof(HWND));//多线程时读取其它线程栈上的数据是不安全的行为
			*hwndcopy=hwnd;
			hthread=(void*)_beginthread(timer,0,(void*)hwndcopy);
		}
		if (first_click)
		{
			initialize(block,k);
			first_click = false;
		}

		if(block[k]&0x80000000&&!(block[k]&0x40000000))
		{
			block[k]&=0x7FFFFFFF;
			if(block[k]==0)
			{
				showaround(block,k);
				InvalidateRect(hwnd,0,0);
			}
			else
			{
				RECT rect={i*size,j*size,i*size+size,j*size+size};
				InvalidateRect(hwnd,&rect,0);
				if(block[k]==9)
				{
					hthread=0;
					ResetEvent(hevent);
					PostMessage(hwnd,WM_USER+1,0,0);
				}
			}
			if(isvictory(block))
			{
				hthread=0;
				ResetEvent(hevent);
				PostMessage(hwnd,WM_USER+1,0,1);
			}
		}
		return 0;
	}
	case WM_RBUTTONDOWN:
	{
		int i=LOWORD(lParam)/size,j=HIWORD(lParam)/size,k=j*30+i;
		if(block[k]&0x80000000)
		{
			if(block[k]&0x40000000)
			{
				block[k]&=0xBFFFFFFF;
				mine++;
			}
			else
			{
				block[k]|=0x40000000;
				mine--;
			}
			RECT rect={i*size,j*size,i*size+size,j*size+size};
			InvalidateRect(hwnd,&rect,0);
		}
		return 0;
	}
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc=BeginPaint(hwnd,&ps);
		SetStretchBltMode(hdc,STRETCH_DELETESCANS);
		HDC hdcm=CreateCompatibleDC(hdc);
		//提高效率
		DeleteObject(SelectObject(hdc,CreateSolidBrush(RGB(120,150,250))));
		for(int i=0;i<16;i++)
			for(int j=0;j<30;j++)
				if(block[i*30+j]&0x80000000)
				{
					if(block[i*30+j]&0x40000000)
					{
						BITMAP bitmap;
						GetObject(hbitmap[9],sizeof(BITMAP),&bitmap);
						SelectObject(hdcm,hbitmap[9]);
						StretchBlt(hdc,j*size,i*size,size,size,hdcm,0,0,bitmap.bmWidth,bitmap.bmHeight,SRCCOPY);
					}
					else
						Rectangle(hdc,j*size,i*size,j*size+size,i*size+size);
				}

		DeleteObject(SelectObject(hdc,GetStockObject(WHITE_BRUSH)));
		for(int i=0;i<16;i++)
			for(int j=0;j<30;j++)
			{
				int k=i*30+j;
				if(!(block[k]&0x80000000))
				{
					if(block[k]==0)
						Rectangle(hdc,j*size,i*size,j*size+size,i*size+size);
					else if(block[k]>=1&&block[k]<=9)
					{
						BITMAP bitmap;
						GetObject(hbitmap[block[k]-1],sizeof(BITMAP),&bitmap);
						SelectObject(hdcm,hbitmap[block[k]-1]);
						StretchBlt(hdc,j*size,i*size,size,size,hdcm,0,0,bitmap.bmWidth,bitmap.bmHeight,SRCCOPY);
					}
				}
			}
		DeleteDC(hdcm);
		EndPaint(hwnd,&ps);
		return 0;
	}
	case WM_USER+1:
	{
		int flag;
		if(lParam)
			flag=MessageBox(hwnd,_T("恭喜你，获得了胜利！\r\n\r\n确定:重新开始游戏\r\n取消:退出游戏"),_T("胜利"),MB_OKCANCEL|MB_ICONASTERISK);
		else
			flag=MessageBox(hwnd,_T("很遗憾，你失败了！\r\n\r\n确定:重新开始游戏\r\n取消:退出游戏"),_T("失败"),MB_OKCANCEL|MB_ICONASTERISK);
		if(flag==IDOK)
		{
			first_click = true;
			initializeunknown(block);
			InvalidateRect(hwnd,0,0);
			HWND hwndparent=GetParent(hwnd);
			//InvalidateRect(hwndparent,0,1);闪屏
			HDC hdc=GetDC(hwndparent);
			erasetext(hwnd,hdc);
			ReleaseDC(hwndparent,hdc);
			time=0;mine=99;
		}
		else
		{
			free(block);
			freebitmap(hbitmap);
			CloseHandle(hevent);
			PostMessage(GetParent(hwnd),WM_DESTROY,0,0);
		}
		return 0;
	}
	}
	return DefWindowProc(hwnd,message,wParam,lParam);
}

void timer(void* Parameter)
{
	HWND hwnd=*(HWND*)Parameter;
	free(Parameter);
	HANDLE htimer=CreateWaitableTimer(0,0,0);
	LARGE_INTEGER li;
	li.QuadPart=-10000000;
	SetWaitableTimer(htimer,&li,1000,0,0,0);
	//HANDLE hevent=OpenEvent(0,0,_T("continue"));//OpenEvent失败,CreateEvent成功???
	HANDLE hevent=CreateEvent(0,1,0,_T("continue"));
	while(WaitForSingleObject(hevent,0)==WAIT_OBJECT_0)
	{
		WaitForSingleObject(htimer,INFINITE);
		SendMessage(hwnd,WM_TIMER,0,0);
	}
}
void erasetext(HWND hwnd,HDC hdc)
{
	RECT rect;
	GetClientRect(hwnd,&rect);
	rect.bottom=20;
	HBRUSH hbrush=CreateSolidBrush(RGB(215,225,235));
	FillRect(hdc,&rect,hbrush);
	DeleteObject(hbrush);
}