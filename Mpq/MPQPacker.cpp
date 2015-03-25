#include "MPQPacker.h"
#include "MPQPackage.h"
#include "RunnableAsync.h"
#include <vector>
#include <Windows.h>
#include <ImageHlp.h>

string stringTrim(const string& str,const string& ch)
{
	string ret = str;
	int pos = ret.find(ch);
	while( pos != string::npos )
	{
		string s1 = ret.substr(0,pos);
		string s2 = ret.substr(pos+ch.length(),ret.length());
		ret = s1+s2;
		pos = ret.find(ch);
		
	}
	return ret;
}

int CountFile(const char* path)
{
	int count = 0;

	string sFileKey = "";
	string sFindKey = path;
	sFindKey.append("/*.*");

	WIN32_FIND_DATA fileData;
	HANDLE hNextFile=::FindFirstFile(sFindKey.c_str(),&fileData);
	if(INVALID_HANDLE_VALUE == hNextFile)
	{
		return 0;
	}

	do{
		if(fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if(fileData.cFileName[0] != '.')
			{
				sFindKey = path;
				sFindKey.append("/");
				sFindKey.append(fileData.cFileName);
				count += CountFile(sFindKey.c_str());
			}
		}
		else
		{
			sFileKey = path;
			sFileKey += "/";
			sFileKey += fileData.cFileName;

			if (std::string::npos != sFileKey.find(".svn"))
			{
				continue;
			}
			else if (	(std::string::npos != sFileKey.find(".jpg")) ||
				(std::string::npos != sFileKey.find(".png")) ||
				(std::string::npos != sFileKey.find(".pvr")))
			{
				
			}
			++count;
		}
	}while(FindNextFile(hNextFile,&fileData));

	FindClose(hNextFile);

	return count;
}

class MPQPackerAsync : public IRunnable
{
public:
	MPQPackerAsync(MPQPacker* pOwner,const char* path,const char* mpq,const char* config) : m_pOwner(pOwner){
		m_sMpqFile = mpq;
		m_sPath = path;
		m_sConfig = config;
	}
	~MPQPackerAsync(){}

protected:
	bool append_file(const char* path,const char* file_name,MPQPackage* pPkg,FILE* fp_log)
	{
 		if ( !path || !file_name || !pPkg)
 		{
 			return false;
 		}
 
		string fullPathKey = stringTrim(path,"\n");
		string filePathKey = stringTrim(file_name,"\n");
		if ( filePathKey.empty() )
		{
			return false;
		}
		if ( fullPathKey.empty() )
		{
			fullPathKey = filePathKey;
		}
		else
		{
			fullPathKey.append("/");
			fullPathKey.append(filePathKey);
		}

 		FILE* fp = fopen(fullPathKey.c_str(),"rb");
 		if ( !fp )
 		{
			if ( fp_log )
			{
				fprintf(fp_log,"can't open file %s\n",fullPathKey.c_str());
			}
 			return false;
 		}
 
 		// md5 check code
 		MD5 m;
 		m.update(fp);
 
 		fseek(fp,0,SEEK_END);
 		unsigned int file_size = ftell(fp);
		m.update(&file_size,sizeof(unsigned int));
 
 		unsigned char* pData = new unsigned char[file_size];
 		if ( !pData )
 		{
			if ( fp_log )
			{
				fprintf(fp_log,"can't assign memory for file %s\n",fullPathKey.c_str());
			}
 			fclose(fp);
 			return false;
 		}
 
 		fseek(fp,0,SEEK_SET);
 		fread(pData,file_size,1,fp);
 		fclose(fp);

		bool bCompress = true;
		if ((std::string::npos != fullPathKey.find(".jpg")) ||
			(std::string::npos != fullPathKey.find(".png")) ||
			(std::string::npos != fullPathKey.find(".pvr")))
		{
			bCompress = false;
		}
 
 		MPQHashNode* pNode = pPkg->get_hash_node_new(filePathKey.c_str());
 		if ( !pNode )
 		{
 			// no space
			if ( fp_log )
			{
				fprintf(fp_log,"no hash node left for file %s\n",filePathKey.c_str());
			}
 			delete pData;
 			return false;
 		}
 
 		uint32 blockIndex = pPkg->append_data(pData,file_size,bCompress);
 		if ( blockIndex == MPQ_BLOCK_INVALID )
 		{
 			pPkg->reset_hash_node(pNode);
 			delete pData;
			if ( fp_log )
			{
				fprintf(fp_log,"append data failed for file %s\n",filePathKey.c_str());
			}
 			return false;
 		}
 
 		pNode->block_index = blockIndex;
 		MPQBlock* pBlock = pPkg->get_block(pNode);
 		memcpy(pBlock->md5,m.result(),MPQ_FILE_MD5_CODE_LEN);
		pBlock->file_name = filePathKey;
 
 		delete pData;

		if ( fp_log )
		{
			fprintf(fp_log,"added file %s \n",filePathKey.c_str());
		}
		
		return true;
	}

	unsigned long pack(const char* path,MPQPackage* pPkg,FILE* fp_log)
	{
		int count = 0;
		string sFileKey = "";
		string sFindKey = path;
		sFindKey.append("/*.*");

		WIN32_FIND_DATA fileData;
		HANDLE hNextFile=::FindFirstFile(sFindKey.c_str(),&fileData);
		if(INVALID_HANDLE_VALUE == hNextFile)
		{
			return 0;
		}

		do{
			if(fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if(fileData.cFileName[0] != '.')
				{
					sFindKey = path;
					sFindKey.append("/");
					sFindKey.append(fileData.cFileName);
					count += pack(sFindKey.c_str(),pPkg,fp_log);
				}
			}
			else
			{      //deal with FindFileData.cFileName	
				sFileKey = path;
				sFileKey += "/";
				sFileKey += fileData.cFileName;

				if (std::string::npos != sFileKey.find(".svn"))
				{
					continue;
				}

				if ( append_file("",sFileKey.c_str(),pPkg,fp_log) )
				{
					++count;
				}

				m_pOwner->setPos(m_pOwner->getPos()+1);
			}
		}while(FindNextFile(hNextFile,&fileData));

		FindClose(hNextFile);

		return count;
	}

	unsigned long pack(const char* path,const char* mpq)
	{
		int file_count = CountFile(path);
		if ( file_count == 0 )
		{
			return 0;
		}

		m_pOwner->setRange(file_count);

		MPQPackage* pPkg = new MPQPackage();
		if ( !pPkg || !pPkg->create(mpq) )
		{
			if ( pPkg )
			{
				delete pPkg;
			}
			return 0;
		}

		FILE* fp_log = fopen("log.txt","w+");

		int ret = pack(path,pPkg,fp_log);

		if ( fp_log )
		{
			fprintf(fp_log,"file count:%d,packed file count:%d\n",file_count,ret);
			fclose(fp_log);
		}

		if ( pPkg )
		{
			pPkg->log_block_table("pack.log");
			pPkg->close();
			delete pPkg;
		}
		
		return ret;
	}

	unsigned long packWithConfig(const char* path,const char* mpq,const char* config)
	{
		FILE* fp_config = fopen(config,"r+");
		if ( !fp_config )
		{
			return 0;
		}

		vector<string> pack_files;
		while( !feof(fp_config) )
		{
			char szBuffer[MAX_PATH] = "";
			fgets(szBuffer,MAX_PATH,fp_config);
			string fileKey = stringTrim(szBuffer,"\n");
			fileKey = stringTrim(fileKey," ");
			if ( !fileKey.empty() )
			{
				pack_files.push_back(fileKey);
			}
		}

		fclose(fp_config);

		MPQPackage* pPkg = new MPQPackage();
		if ( !pPkg || !pPkg->create(mpq) )
		{
			if ( pPkg )
			{
				delete pPkg;
			}
			return 0;
		}

		FILE* fp_log = fopen("log.txt","w+");

		int pack_count = 0;
		m_pOwner->setRange((int)pack_files.size());
		for ( size_t i = 0;i< pack_files.size(); ++i )
		{
			if( append_file(path,pack_files[i].c_str(),pPkg,fp_log) )
			{
				++pack_count;
			}
			m_pOwner->setPos(i+1);
		}

		if ( fp_log )
		{
			fprintf(fp_log,"file count:%d,packed file count:%d\n",pack_files.size(),pack_count);
			fclose(fp_log);
		}

		if ( pPkg )
		{
			pPkg->log_block_table("packWithConfig.log");
			pPkg->close();
			delete pPkg;
		}

		return pack_count;
	}

	void execute(){
		if ( m_sPath.empty() || m_sMpqFile.empty())
		{
			return;
		}

		if ( m_sConfig.empty() )
		{
			pack(m_sPath.c_str(),m_sMpqFile.c_str());
		}
		else
		{
			packWithConfig(m_sPath.c_str(),m_sMpqFile.c_str(),m_sConfig.c_str());
		}
		

		m_pOwner->onComplete();
	}

	MPQPacker* m_pOwner;
	string m_sMpqFile;
	string m_sPath;
	string m_sConfig;
};


MPQPacker::MPQPacker() : m_bFree(true)
{

}

MPQPacker::~MPQPacker()
{

}

bool MPQPacker::pack(const char* path,const char* mpq)
{
	if ( isBusy() )
	{
		return false;
	}
	reset();
	doAsync(new MPQPackerAsync(this,path,mpq,""));
	m_bFree = false;

	return true;
}

bool MPQPacker::packWithConfig(const char* path,const char* config,const char* mpq)
{
	if ( isBusy() )
	{
		return false;
	}
	reset();
	doAsync(new MPQPackerAsync(this,path,mpq,config));
	m_bFree = false;

	return true;
}

void MPQPacker::onComplete()
{
	m_bFree = true;
}