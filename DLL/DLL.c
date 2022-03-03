#define export __declspec(dllexport)
#include "DLL.h"
#include "resource.h"

BOOL APIENTRY DllMain(HMODULE hModule,DWORD  ul_reason_for_call,LPVOID lpReserved)
{
	switch(ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

void initialize(int* block)
{
	FILETIME filetime;
	GetSystemTimeAsFileTime(&filetime);
	srand(filetime.dwLowDateTime);
	memset(block,0,480*sizeof(int));
	for(int i=0;i<99;i++)
	{
		int j=rand()%480;
		if(block[j]!=9)
			block[j]=9;
		else
			i--;
	}
	for(int i=0; i<480; i++)
	{
		count(block,i);
		block[i]|=0x80000000;
	}
}

void count(int* block,int num)
{
	int i=num%30,j=num/30,k=0;//i为列数,j为行数
	if(block[num]!=9)
	{
		if(i!=0&&j!=0&&(block[num-31]&0xF)==9)
			k++;
		if(i!=0&&(block[num-1]&0xF)==9)
			k++;
		if(i!=0&&j!=15&&(block[num+29]&0xF)==9)
			k++;
		if(j!=0&&(block[num-30]&0xF)==9)
			k++;
		if(j!=15&&(block[num+30]&0xF)==9)
			k++;
		if(i!=29&&j!=0&&(block[num-29]&0xF)==9)
			k++;
		if(i!=29&&(block[num+1]&0xF)==9)
			k++;
		if(i!=29&&j!=15&&(block[num+31]&0xF)==9)
			k++;
		block[num]=k;
	}
}

void showaround(int* block,int num)
{
	int i=num%30,j=num/30;//i为列数,j为行数
	if(i!=0&&j!=0&&(block[num-31]&0x80000000))
	{
		block[num-31]&=0x7FFFFFFF;
		if(block[num-31]==0)
			showaround(block,num-31);
	}
	if(i!=0&&(block[num-1]&0x80000000))
	{
		block[num-1]&=0x7FFFFFFF;
		if(block[num-1]==0)
			showaround(block,num-1);
	}
	if(i!=0&&j!=15&&(block[num+29]&0x80000000))
	{
		block[num+29]&=0x7FFFFFFF;
		if(block[num+29]==0)
			showaround(block,num+29);
	}
	if(j!=0&&(block[num-30]&0x80000000))
	{
		block[num-30]&=0x7FFFFFFF;
		if(block[num-30]==0)
			showaround(block,num-30);
	}
	if(j!=15&&(block[num+30]&0x80000000))
	{
		block[num+30]&=0x7FFFFFFF;
		if(block[num+30]==0)
			showaround(block,num+30);
	}
	if(i!=29&&j!=0&&(block[num-29]&0x80000000))
	{
		block[num-29]&=0x7FFFFFFF;
		if(block[num-29]==0)
			showaround(block,num-29);
	}
	if(i!=29&&(block[num+1]&0x80000000))
	{
		block[num+1]&=0x7FFFFFFF;
		if(block[num+1]==0)
			showaround(block,num+1);
	}
	if(i!=29&&j!=15&&(block[num+31]&0x80000000))
	{
		block[num+31]&=0x7FFFFFFF;
		if(block[num+31]==0)
			showaround(block,num+31);
	}
}

int isvictory(int* block)
{
	for(int i=0;i<480;i++)
		if((block[i]&0x80000000)&&((block[i]&0xF)!=9))
			return 0;
	return 1;
}

HBITMAP* loadbitmap()
{
	HBITMAP* hbitmap=calloc(10,sizeof(HBITMAP));
	TCHAR buffer[]=_T("1.bmp");
	for(int i=0;i<8;i++)
	{
		hbitmap[i]=LoadImage(0,buffer,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
		buffer[0]+=1;
	}
	hbitmap[8]=LoadImage(0,_T("mine.bmp"),IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
	hbitmap[9]=LoadImage(0,_T("redflag.bmp"),IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
	HINSTANCE hInstance=GetModuleHandle(_T("DLL.dll"));
	for(int i=0;i<10;i++)
		if(!hbitmap[i])
			hbitmap[i]=LoadImage(hInstance,(TCHAR*)(IDB_BITMAP1+i),IMAGE_BITMAP,0,0,0);
	return hbitmap;
}

void freebitmap(HBITMAP* hbitmap)
{
	for(int i=0;i<10;i++)
		DeleteObject(hbitmap[i]);
	free(hbitmap);
}