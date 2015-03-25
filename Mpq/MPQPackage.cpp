#include "MPQPackage.h"
#include "hashCode.h"
#include <memory.h>
#include "zlib/zlib.h"

MPQPackage::MPQPackage() : m_fptr(NULL),m_IdentCode(0)
{
	clear_block_table();
}

MPQPackage::~MPQPackage()
{
	clear_block_table();
}

void MPQPackage::clear_block_table()
{
	for ( size_t i=0;i<m_blockTable.size();++i )
	{
		delete m_blockTable[i];
	}

	m_blockTable.clear();
}

void MPQPackage::log_block_table(const char* log)
{
	FILE* fp = fopen(log,"w+");
	if ( !fp )
	{
		return ;
	}

	for ( size_t i=0;i<m_blockTable.size();++i )
	{
		MPQBlock* pBlock = m_blockTable[i];
		if ( pBlock )
		{
			fprintf(fp,"idx:%04u,md5:%s,file: %s\n",i+1,MD5::binaryToHexString(pBlock->md5,MPQ_FILE_MD5_CODE_LEN).c_str(),pBlock->file_name.c_str());
		}
	}

	fclose(fp);
}

void MPQPackage::read_header()
{
	if ( !m_fptr )
	{
		return ;
	}

	fseek(m_fptr,0,SEEK_SET);
	fread(&m_stHeader,sizeof(MPQHeader),1,m_fptr);
}

void MPQPackage::read_hash_table()
{
	if ( !m_fptr )
	{
		return ;
	}

	fseek(m_fptr,m_stHeader.hash_table_offset,SEEK_SET);
	fread(m_hashTable,sizeof(MPQHashNode),m_stHeader.hash_count,m_fptr);
}

void MPQPackage::read_block_table()
{
	if ( !m_fptr )
	{
		return ;
	}

	clear_block_table();

	fseek(m_fptr,m_stHeader.block_table_offset,SEEK_SET);

	for ( uint32 i = 0;i<m_stHeader.block_count; ++i )
	{
		MPQBlock* pBlock = new MPQBlock;
		uchar len = 0;
		fread(&len,1,1,m_fptr);
		char* name = new char[len+1];
		memset(name,0,len+1);
		fread(name,len,1,m_fptr);
		pBlock->file_name = name;
		delete name;
		fread(&(pBlock->offset),sizeof(uint32),1,m_fptr);
		fread(&(pBlock->size),sizeof(uint32),1,m_fptr);
		fread(&(pBlock->uncompress_size),sizeof(uint32),1,m_fptr);
		fread(&(pBlock->flag),sizeof(uint32),1,m_fptr);
		fread(&(pBlock->md5),MPQ_FILE_MD5_CODE_LEN,1,m_fptr);
		m_blockTable.push_back(pBlock);
	}
}

void MPQPackage::write_header()
{
	if ( !m_fptr )
	{
		return ;
	}

	fseek(m_fptr,0,SEEK_SET);
	fwrite(&m_stHeader,sizeof(MPQHeader),1,m_fptr);
}

void MPQPackage::write_hash_table()
{
	if ( !m_fptr )
	{
		return ;
	}

	fseek(m_fptr,m_stHeader.hash_table_offset,SEEK_SET);
	fwrite(m_hashTable,sizeof(MPQHashNode),m_stHeader.hash_count,m_fptr);
}

void MPQPackage::write_block_table()
{
	if ( !m_fptr )
	{
		return ;
	}

	fseek(m_fptr,m_stHeader.block_table_offset,SEEK_SET);

	for ( uint32 i = 0;i<m_blockTable.size(); ++i )
	{
		MPQBlock* pBlock = m_blockTable[i];
		uchar len = (uchar)pBlock->file_name.length();
		fwrite(&len,1,1,m_fptr);
		fwrite(pBlock->file_name.c_str(),len,1,m_fptr);
		fwrite(&(pBlock->offset),sizeof(uint32),1,m_fptr);
		fwrite(&(pBlock->size),sizeof(uint32),1,m_fptr);
		fwrite(&(pBlock->uncompress_size),sizeof(uint32),1,m_fptr);
		fwrite(&(pBlock->flag),sizeof(uint32),1,m_fptr);
		fwrite(&(pBlock->md5),MPQ_FILE_MD5_CODE_LEN,1,m_fptr);
	}
}

bool MPQPackage::create(const char* file_name)
{
	m_fptr = fopen(file_name,"wb+");
	if ( !m_fptr )
	{
		return false;
	}

	m_IdentCode = RSHash(file_name);
	m_stHeader.header_size = sizeof(MPQHeader);
	m_stHeader.hash_table_offset = m_stHeader.header_size+m_stHeader.data_size;
	m_stHeader.block_table_offset = m_stHeader.hash_table_offset+sizeof(m_hashTable);
	m_stHeader.format_version1 = 0;
	m_stHeader.format_version2 = 0;

	write_header();
	write_hash_table();
	write_block_table();

	return true;
}

bool MPQPackage::open(const char* file_name)
{
	m_fptr = fopen(file_name,"rb+");
	if ( !m_fptr )
	{
		return false;
	}
	m_IdentCode = RSHash(file_name);
	read_header();
	if( m_stHeader.sign[0] != 'M' || m_stHeader.sign[1] != 'P' || 
		m_stHeader.sign[2] != 'Q' || m_stHeader.sign[3] != 0x1A )
	{
		fclose(m_fptr);
		m_fptr = NULL;
		return false;
	}

	read_hash_table();
	read_block_table();

	pthread_mutex_init(&m_readMutex, NULL);

	return true;
}

bool MPQPackage::backup(const char* file_name)
{
	if ( !m_IdentCode )
	{
		return false;
	}

	FILE* fp = fopen(file_name,"wb+");
	if ( !fp )
	{
		return false;
	}

	// 文件标识
	fwrite(&m_IdentCode,sizeof(m_IdentCode),1,fp);
	// 文件头
	fwrite(&m_stHeader,sizeof(MPQHeader),1,fp);
	// 文件hash表
	fwrite(m_hashTable,sizeof(MPQHashNode),m_stHeader.hash_count,fp);
	// 文件块表
	for ( uint32 i = 0;i<m_blockTable.size(); ++i )
	{
		MPQBlock* pBlock = m_blockTable[i];
		uchar len = (uchar)pBlock->file_name.length();
		fwrite(&len,1,1,fp);
		fwrite(pBlock->file_name.c_str(),len,1,fp);
		fwrite(&(pBlock->offset),sizeof(uint32),1,fp);
		fwrite(&(pBlock->size),sizeof(uint32),1,fp);
		fwrite(&(pBlock->uncompress_size),sizeof(uint32),1,fp);
		fwrite(&(pBlock->flag),sizeof(uint32),1,fp);
		fwrite(&(pBlock->md5),MPQ_FILE_MD5_CODE_LEN,1,fp);
	}
	fclose(fp);

	return true;
}

bool MPQPackage::revert(const char* file_name,const char* file_revert)
{
	m_fptr = fopen(file_name,"rb+");
	if ( !m_fptr )
	{
		return false;
	}
	m_IdentCode = RSHash(file_name);
	read_header();
	if( m_stHeader.sign[0] != 'M' || m_stHeader.sign[1] != 'P' || 
		m_stHeader.sign[2] != 'Q' || m_stHeader.sign[3] != 0x1A )
	{
		fclose(m_fptr);
		m_fptr = NULL;
		return false;
	}

	FILE* fp = fopen(file_revert,"rb+");
	if ( !fp )
	{
		return false;
	}

	unsigned long identCode = 0;
	fread(&identCode,sizeof(identCode),1,fp);
	if ( m_IdentCode != identCode )
	{
		fclose(fp);
		return false;
	}

	// 文件头
	fread(&m_stHeader,sizeof(MPQHeader),1,fp);
	// 文件hash表
	fread(m_hashTable,sizeof(MPQHashNode),m_stHeader.hash_count,fp);
	// 文件块表
	for ( uint32 i = 0;i<m_stHeader.block_count; ++i )
	{
		MPQBlock* pBlock = new MPQBlock;
		uchar len = 0;
		fread(&len,1,1,fp);
		char* name = new char[len+1];
		memset(name,0,len+1);
		fread(name,len,1,fp);
		pBlock->file_name = name;
		delete name;
		fread(&(pBlock->offset),sizeof(uint32),1,fp);
		fread(&(pBlock->size),sizeof(uint32),1,fp);
		fread(&(pBlock->uncompress_size),sizeof(uint32),1,fp);
		fread(&(pBlock->flag),sizeof(uint32),1,fp);
		fread(&(pBlock->md5),MPQ_FILE_MD5_CODE_LEN,1,fp);
		m_blockTable.push_back(pBlock);
	}

	fclose(fp);

	close();

	return true;
}

void MPQPackage::close(bool bSave)
{
	if ( m_fptr )
	{
		if ( bSave )
		{
			write_header();
			write_hash_table();
			write_block_table();
		}

		clear_block_table();
		fclose(m_fptr);
	}
}

bool MPQPackage::is_block_valid(uint32 idx)
{
	return (idx != MPQ_BLOCK_INVALID) && (idx < m_stHeader.block_count);
}

bool MPQPackage::is_hash_node_hashed(uint32 idx)
{
	if( m_hashTable[idx].verify_code_a == MPQ_VERIFY_CODE_INVALID || 
		m_hashTable[idx].verify_code_b == MPQ_VERIFY_CODE_INVALID ){
			return false;
	}
	return true;
}

bool MPQPackage::is_hash_node_valid(uint32 idx)
{
	if( m_hashTable[idx].verify_code_a == MPQ_VERIFY_CODE_INVALID || 
		m_hashTable[idx].verify_code_b == MPQ_VERIFY_CODE_INVALID ||
		m_hashTable[idx].block_index == MPQ_BLOCK_INVALID ){
			return false;
	}
	return true;
}

bool MPQPackage::is_hash_node_valid(MPQHashNode* pNode)
{
	if(!pNode ||
		pNode->verify_code_a == MPQ_VERIFY_CODE_INVALID || 
		pNode->verify_code_b == MPQ_VERIFY_CODE_INVALID ||
		pNode->block_index == MPQ_BLOCK_INVALID ){
			return false;
	}
	return true;
}

MPQHashNode* MPQPackage::get_hash_node(uint32 idx)
{
	idx = idx % MPQ_HASH_TABLE_SIZE;
	return &m_hashTable[idx];
}

MPQHashNode* MPQPackage::get_hash_node_valid(uint32 hash_code,uint32 verify_a,uint32 verify_b)
{
	uint32 idx = hash_code % MPQ_HASH_TABLE_SIZE;
	uint32 nHash0 = idx;
	while ( true )
	{
		if (m_hashTable[idx].verify_code_a == verify_a && 
			m_hashTable[idx].verify_code_b == verify_b )
		{
			if ( is_hash_node_valid( &m_hashTable[idx] ) )
			{
				return &m_hashTable[idx];
			}

			return NULL;
			
		}
		else
		{
			idx = (idx + 1) % MPQ_HASH_TABLE_SIZE;
		}

		if (idx == nHash0)
			break;
	}

	return NULL;
}

MPQHashNode* MPQPackage::get_hash_node_valid(const char* file_name)
{   
	unsigned long nHash0 = BlizzardHash(file_name);
	unsigned long nHashA = BlizzardHashA(file_name);
	unsigned long nHashB = BlizzardHashB(file_name);

	return get_hash_node_valid(nHash0,nHashA,nHashB);
}

MPQHashNode* MPQPackage::get_hash_node_new_for_diff(uint32 hash_code,uint32 verify_a,uint32 verify_b)
{
	hash_code = hash_code % MPQ_HASH_TABLE_SIZE;
	unsigned long nIndex = hash_code;
	while ( is_hash_node_hashed(nIndex) )
	{
		nIndex = (nIndex + 1) % MPQ_HASH_TABLE_SIZE;
		if (nIndex == hash_code)
		{
			return NULL;
		}
	}

	m_hashTable[nIndex].verify_code_a = verify_a;
	m_hashTable[nIndex].verify_code_b = verify_b;
	return &m_hashTable[nIndex];
}

MPQHashNode* MPQPackage::get_hash_node_new(uint32 hash_code,uint32 verify_a,uint32 verify_b)
{
	hash_code = hash_code % MPQ_HASH_TABLE_SIZE;
	unsigned long nIndex = hash_code;
	while ( is_hash_node_valid(nIndex) )
	{
		nIndex = (nIndex + 1) % MPQ_HASH_TABLE_SIZE;
		if (nIndex == hash_code)
		{
			return NULL;
		}
	}

	m_hashTable[nIndex].verify_code_a = verify_a;
	m_hashTable[nIndex].verify_code_b = verify_b;
	return &m_hashTable[nIndex];
}

MPQHashNode* MPQPackage::get_hash_node_new(const char* file_name)
{
	unsigned long nHash0 = BlizzardHash(file_name);
	unsigned long nHashA = BlizzardHashA(file_name);
	unsigned long nHashB = BlizzardHashB(file_name);

	return get_hash_node_new(nHash0,nHashA,nHashB);
}

void MPQPackage::reset_hash_node(MPQHashNode* pNode)
{
	pNode->verify_code_a = MPQ_VERIFY_CODE_INVALID;
	pNode->verify_code_b = MPQ_VERIFY_CODE_INVALID;
	pNode->block_index = MPQ_BLOCK_INVALID;
}

MPQBlock* MPQPackage::get_block(const char* file_name)
{
	MPQHashNode* pNode = get_hash_node_valid(file_name);
	return get_block(pNode);
}

MPQBlock* MPQPackage::get_block(MPQHashNode* pNode)
{
	if ( !pNode || !is_block_valid(pNode->block_index) )
	{
		return NULL;
	}

	return m_blockTable[pNode->block_index];
}

unsigned char* MPQPackage::read_block(MPQBlock* pBlock,uint32& size)
{
	if ( !pBlock )
	{
		size = 0;
		return NULL;
	}

	unsigned char* pData = new unsigned char[pBlock->size];
	if ( !pData )
	{
		size = 0;
		return NULL;
	}

	size = pBlock->size;

	pthread_mutex_lock(&m_readMutex);
	fseek(m_fptr,pBlock->offset,SEEK_SET);
	fread(pData,pBlock->size,1,m_fptr);
	pthread_mutex_unlock(&m_readMutex);

	unsigned char* pOut = pData;

	if ( pBlock->is_mark(MPQ_FILE_COMPRESS) )
	{
		uLongf uncompress_size = pBlock->uncompress_size;
		pOut = new uchar[pBlock->uncompress_size];

		if ( Z_OK != uncompress(pOut,&uncompress_size,pData,pBlock->size) )
		{
			delete pData;
			delete pOut;
			return NULL;
		}

		size = uncompress_size;
	}

	if ( pBlock->is_mark(MPQ_FILE_ENCRYPT) )
	{

	}

	return pOut;
}

unsigned char* MPQPackage::read_node(MPQHashNode* pNode,uint32& size)
{
	if ( !pNode )
	{
		size = 0;
		return NULL;
	}

	return read_block(get_block(pNode),size);
}

unsigned char* MPQPackage::read_file(const char* file_name,uint32& size)
{
	if ( !m_fptr )
	{
		size = 0;
		return NULL;
	}

	return read_block(get_block(file_name),size);
}

uint32 MPQPackage::append_data(const uchar* pData,uint32 size,bool bCompress,bool bEncrypt)
{
	if ( !m_fptr || !pData || size == 0 )
	{
		return MPQ_BLOCK_INVALID;
	}

	MPQBlock* pBlock = new MPQBlock;
	pBlock->uncompress_size = size;
	pBlock->size = size;
	pBlock->offset = m_stHeader.header_size+m_stHeader.data_size;

	pBlock->mark(MPQ_FILE_EXISTS);

	if ( bEncrypt )
	{
		pBlock->mark(MPQ_FILE_ENCRYPT);
	}

	if ( bCompress && size > MPQ_COMPRESS_SIZE)
	{
		uLongf compress_size = size;
		uchar* pBuf = new uchar[size];
		if ( Z_OK != compress((Bytef*)pBuf,&compress_size,pData,size) )
		{
			delete pBuf;
			delete pBlock;
			return MPQ_BLOCK_INVALID;
		}
		pBlock->size = compress_size;
		pBlock->mark(MPQ_FILE_COMPRESS);

		fseek(m_fptr,pBlock->offset,SEEK_SET);
		fwrite(pBuf,pBlock->size,1,m_fptr);

		delete pBuf;
	}
	else
	{
		fseek(m_fptr,pBlock->offset,SEEK_SET);
		fwrite(pData,pBlock->size,1,m_fptr);
	}

	m_stHeader.data_size += pBlock->size;
	m_stHeader.hash_table_offset = m_stHeader.header_size+m_stHeader.data_size;
	m_stHeader.block_table_offset = m_stHeader.hash_table_offset+sizeof(m_hashTable);

	m_blockTable.push_back(pBlock);
	m_stHeader.block_count = m_blockTable.size();

	return m_stHeader.block_count-1;
}

void MPQPackage::remove_data(MPQHashNode* pNode)
{
	if ( pNode )
	{
		remove_data(get_block(pNode));
		reset_hash_node(pNode);
	}
}

void MPQPackage::remove_data(MPQBlock* pBlock)
{
	if ( pBlock && pBlock->is_mark(MPQ_FILE_EXISTS) )
	{
		pBlock->cl_mark(MPQ_FILE_EXISTS);
		m_stHeader.hole_size += pBlock->size;
	}
}

void MPQPackage::remove_data(uint32 idx)
{
	if( !is_block_valid(idx) )
	{
		return ;
	}
	
	remove_data(m_blockTable[idx]);
}

bool MPQPackage::append_file(const char* file_name,bool bCompress,bool bEncrypt)
{
	if ( !file_name )
	{
		return false;
	}

	FILE* fp = fopen(file_name,"rb");
	if ( !fp )
	{
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
		fclose(fp);
		return false;
	}

	fseek(fp,0,SEEK_SET);
	fread(pData,file_size,1,fp);
	fclose(fp);

	MPQHashNode* pNode = get_hash_node_new(file_name);
	if ( !pNode )
	{
		// no space
		delete pData;
		return false;
	}

	uint32 blockIndex = append_data(pData,file_size,bCompress,bEncrypt);
	if ( blockIndex == MPQ_BLOCK_INVALID )
	{
		reset_hash_node(pNode);
		delete pData;
		return false;
	}

	pNode->block_index = blockIndex;
	MPQBlock* pBlock = get_block(pNode);
	memcpy(pBlock->md5,m.result(),MPQ_FILE_MD5_CODE_LEN);
	pBlock->file_name = file_name;
	
	delete pData;
	return true;
}

void MPQPackage::remove_file(const char* file_name)
{
	MPQHashNode* pNode = get_hash_node_valid(file_name);
	if ( pNode )
	{
		remove_data(pNode);

		write_header();
		write_hash_table();
		write_block_table();
	}
}
