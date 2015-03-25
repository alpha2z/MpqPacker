#include <stdio.h>
#include <conio.h>
#include <string>
#include <Windows.h>
using namespace std;

#include "MPQPacker.h"
#include "MPQPatcher.h"
#include "MPQPackage.h"

void main()
{
	string sTips = "";

	sTips += "/**************************************************************/\n";
	sTips += "/************* 欢迎使用广州魔君工作室资源打包工具 ***************/\n";
	sTips += "/**************************************************************/\n";
	sTips += "/************* 版权：广州魔君工作室 *******************/\n";
	sTips += "/**************************************************************/\n";
	printf("%s",sTips.c_str());

	FILE* fp = fopen("gamepacker.conf","r");
	if ( !fp )
	{
		printf("config file gamepacker.conf not found!\n");
		printf("press any key to exit!\n");
		getch();
		return ;
	}

	char pack1[260] = "";
	char pack2[260] = "";
	char currDir[260] = "";

	GetCurrentDirectory(260,currDir);
	printf("current directory:%s\n",currDir);

	printf("gamepacker.conf:\n");
	fgets(pack1,260,fp);
	printf("%s",pack1);
	fgets(pack2,260,fp);
	printf("%s",pack2);

	string pack1PathKey = pack1;
	string pack2PathKey = pack2;
	string currPathKey = currDir;

	pack1PathKey.erase(0,pack1PathKey.find_last_of("=")+1);
	pack2PathKey.erase(0,pack2PathKey.find_last_of("=")+1);

	pack1PathKey.erase(pack1PathKey.find_last_not_of("\n")+1);
	pack2PathKey.erase(pack2PathKey.find_last_not_of("\n")+1);

	int counter = 0;
	MPQPacker packer;
	// pack1
	SetCurrentDirectory(currPathKey.c_str());
	SetCurrentDirectory(pack1PathKey.c_str());
	string targetFilePack1 = currPathKey+"/pack1.mpq";

	printf("\n\npark1 start!\n");
	packer.pack("assets",targetFilePack1.c_str());
	while(packer.isBusy())
	{
		if ( counter++ % 10 == 0)
		{
			printf(".");
		}
		Sleep(50);
	}
	printf("\npark1 complete!\n\n");
	counter = 0;

	// pack2
	SetCurrentDirectory(currPathKey.c_str());
	SetCurrentDirectory(pack2PathKey.c_str());
	string targetFilePack2 = currPathKey+"/pack2.mpq";

	printf("park2 start!\n");
	packer.pack("assets",targetFilePack2.c_str());
	while(packer.isBusy())
	{
		if ( counter++ % 10 == 0)
		{
			printf(".");
		}
		Sleep(50);
	}
	printf("\npark2 complete!\n\n");
	counter = 0;

	SetCurrentDirectory(currPathKey.c_str());
	MPQPackage* pPkg = new MPQPackage;
	if ( pPkg && pPkg->create("data.mpq"))
	{
		pPkg->close();
		delete pPkg;
		pPkg = NULL;
		// patch total mpq
		MPQPatcher patcher;

		printf("patching pack1 start!\n");
		patcher.patch("data.mpq",targetFilePack1.c_str(),"data.bak");   //第三个参数自己加上的;
		while(patcher.isBusy())
		{
			if ( counter++ % 10 == 0)
			{
				printf(".");
			}
			Sleep(50);
		}
		printf("\npatch pack1 complete!\n\n");
		counter = 0;

		printf("patching pack2 start!\n");
		patcher.patch("data.mpq",targetFilePack2.c_str(),"data.bak");  //第三个参数自己加上的;
		while(patcher.isBusy())
		{
			if ( counter++ % 10 == 0)
			{
				printf(".");
			}
			Sleep(50);
		}
		printf("\npatch pack2 complete!\n");
		counter = 0;
	}
	else
	{
		printf("create data.mpq failed!\n");
	}

	if ( pPkg )
	{
		pPkg->close();
		delete pPkg;
	}

	MPQPackage* pPack1 = new MPQPackage;
	MPQPackage* pPack2 = new MPQPackage;
	MPQPackage* pData = new MPQPackage;
	if ( pPack1 && pPack1->open("pack1.mpq") )
	{
		pPack1->log_block_table("pack1.log");
	}

	if ( pPack2 && pPack2->open("pack2.mpq") )
	{
		pPack2->log_block_table("pack2.log");
	}

	if ( pData && pData->open("data.mpq") )
	{
		pData->log_block_table("data.log");
	}

	if ( pPack1 )
	{
		pPack1->close();
		delete pPack1;
	}

	if ( pPack2 )
	{
		pPack2->close();
		delete pPack2;
	}

	if ( pData )
	{
		pData->close();
		delete pData;
	}

	printf("\ncomplete! press any key to exit!");
	getch();

}