#pragma once
#include <Windows.h>
#include <tchar.h>

#ifdef export
#undef export
#ifdef __cplusplus
#define export __declspec(dllexport) extern "C"
#else
#define export __declspec(dllexport)
#endif // __cplusplus
#else
#ifdef __cplusplus
#define export __declspec(dllimport) extern "C"
#else
#define export __declspec(dllimport)
#endif // __cplusplus
#endif // export

export HBITMAP* loadbitmap();
export void freebitmap(HBITMAP* hbitmap);

export int isvictory(int* block);
export void initialize(int* block);
export void count(int* block,int num);
export void showaround(int* block,int num);