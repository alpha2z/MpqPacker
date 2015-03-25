#include <stdio.h>
#include <conio.h>
#include "hashCode.h"
#include "MPQPackage.h"
#include <Windows.h>
#include <string>
using namespace std;
#include <ImageHlp.h>

#include "MPQPacker.h"

MPQPackage g_mpqPackage;

void find(const char *lpPath)
{
	char szFind[256];
	char szFile[256];

	WIN32_FIND_DATA FindFileData;

	string xlsFile = "";

	strcpy(szFind,lpPath);
	strcat(szFind,"/*.*");

	HANDLE hFind=::FindFirstFile(szFind,&FindFileData);
	if(INVALID_HANDLE_VALUE == hFind)    return;

	while(TRUE)
	{
		if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if(FindFileData.cFileName[0]!='.')
			{
				strcpy(szFile,lpPath);
				strcat(szFile,"/");
				strcat(szFile,FindFileData.cFileName);
				find(szFile);
			}
		}
		else
		{      //deal with FindFileData.cFileName	
			xlsFile += lpPath;
			xlsFile += "/";
			xlsFile += FindFileData.cFileName;

			if (std::string::npos != xlsFile.find(".svn"))
			{
				continue;
			}
			else if ((std::string::npos != xlsFile.find(".jpg")) ||
				(std::string::npos != xlsFile.find(".png")) ||
				(std::string::npos != xlsFile.find(".pvr")))
			{
				g_mpqPackage.append_file(xlsFile.c_str());
			}
			else
			{
				g_mpqPackage.append_file(xlsFile.c_str(),true);
			}

			printf("added file %s \n",xlsFile.c_str());
			
			xlsFile = "";
		}
		if(!FindNextFile(hFind,&FindFileData))    break;
	}
	FindClose(hFind);
}

#pragma comment(lib, "Dbghelp.lib")

int dump(unsigned int code, struct _EXCEPTION_POINTERS *ep)
{
	HANDLE hFile = CreateFile(TEXT("d:\\mini.dmp"), 
		GENERIC_WRITE, 
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return EXCEPTION_CONTINUE_SEARCH;
	}
	MINIDUMP_EXCEPTION_INFORMATION    exceptioninfo;
	exceptioninfo.ExceptionPointers = ep;
	exceptioninfo.ThreadId          = GetCurrentThreadId();
	exceptioninfo.ClientPointers    = FALSE;

	if (!MiniDumpWriteDump(GetCurrentProcess(),
		GetCurrentProcessId(),
		hFile,
		MiniDumpNormal,
		&exceptioninfo,
		NULL,
		NULL))
	{
		return EXCEPTION_CONTINUE_SEARCH;
	}
	CloseHandle(hFile);
	return EXCEPTION_EXECUTE_HANDLER;
}

void pack()
{
	MPQPacker packer;
	packer.pack("F:\\mpq/Debug/assets","1.mpq");

	while(packer.isBusy())
	{
		Sleep(10);
	}
}

void main()
{
	pack();
// 	if ( !g_mpqPackage.create("data.mpq") )
// 	{
// 		return ;
// 	}
// 
////  	MPQPackage pkg;
////  	if ( !pkg.open("2.mpq") )
////  	{
////  		return ;
////  	}
// 
// 	find("assets");
// 	//g_mpqPackage.patch(&pkg);
//
//	printf("\ncompleted! press any key exit!");
//	getch();
// 
//  	uint32 size = 0;
//  	const unsigned char* pData = g_mpqPackage.read_file("assets/map/yinxingshangu.tmf",size);
// 
// 	g_mpqPackage.close();


// 	__try
//     {
// 		MPQPackage* pkg = NULL;
// 		pkg->open("");
//     }
//     __except(dump(GetExceptionCode(), GetExceptionInformation()))
//     {
//         TerminateProcess(GetCurrentProcess(), GetExceptionCode());
//     }
}