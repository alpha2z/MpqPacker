#ifndef __MPQPackage_h_
#define __MPQPackage_h_

#include "common.h"
#include <stdio.h>
#include <vector>
#include <string>
#include "pthread.h"
#include "md5Code.h"

using namespace std;

#define MPQ_BLOCK_INVALID		-1
#define MPQ_VERIFY_CODE_INVALID -1
#define MPQ_HASH_CODE_INVALID	0x4000

#define MPQ_FILE_MD5_CODE_LEN	16

#define MPQ_HASH_TABLE_SIZE		0x4FFF

#define MPQ_FILE_ENCRYPT		0x01
#define MPQ_FILE_COMPRESS		0x02
#define MPQ_FILE_EXISTS			0x04

#define MPQ_COMPRESS_SIZE		0x40

typedef struct _mpq_header_
{
	uchar	sign[4];
	uint32	header_size;
	uint32	data_size;
	uint32	hole_size;
	uint16	format_version1;
	uint16	format_version2;
	uint32	hash_table_offset;
	uint32	block_table_offset;
	uint32	hash_count;
	uint32	block_count;

	_mpq_header_(){
		sign[0] = 'M';
		sign[1] = 'P';
		sign[2] = 'Q';
		sign[3] = 0x1A;

		header_size = 0;
		data_size = 0;
		hole_size = 0;
		format_version1 = 0;
		format_version2 = 0;
		hash_table_offset = 0;
		block_table_offset = 0;
		hash_count = MPQ_HASH_TABLE_SIZE;
		block_count = 0;
	}
}MPQHeader;

typedef struct _mpq_block_
{
	string file_name;
	uint32 offset;
	uint32 size;
	uint32 uncompress_size;
	uint32 flag;
	uchar md5[MPQ_FILE_MD5_CODE_LEN];

	_mpq_block_(){
		file_name = "";
		offset = 0;
		size = 0;
		uncompress_size = 0;
		flag = 0;
		memset(md5,0,MPQ_FILE_MD5_CODE_LEN);
	}

	void mark(uint32 f)
	{
		flag |= f;
	}

	bool is_mark(uint32 f){
		return (flag & f) != 0;
	}

	void cl_mark(uint32 f)
	{
		flag &= ~f;
	}
}MPQBlock;

typedef struct _mpq_hash_node_
{
	uint32 verify_code_a;
	uint32 verify_code_b;
	uint32 block_index;

	_mpq_hash_node_(){
		verify_code_a = MPQ_VERIFY_CODE_INVALID;
		verify_code_b = MPQ_VERIFY_CODE_INVALID;
		block_index = MPQ_BLOCK_INVALID;
	}

}MPQHashNode;

class MPQPackage
{
public:
	MPQPackage();
	~MPQPackage();

public:
	MPQHashNode*	get_hash_node(uint32 idx);
	MPQHashNode*	get_hash_node_new(uint32 hash_code,uint32 verify_a,uint32 verify_b);
	MPQHashNode*	get_hash_node_new_for_diff(uint32 hash_code,uint32 verify_a,uint32 verify_b);
	MPQHashNode*	get_hash_node_new(const char* file_name);
	MPQHashNode*	get_hash_node_valid(const char* file_name);
	MPQHashNode*	get_hash_node_valid(uint32 hash_code,uint32 verify_a,uint32 verify_b);

	MPQBlock*		get_block(const char* file_name);
	MPQBlock*		get_block(MPQHashNode* pNode);

	/************************************************************************/
	/* 创建一个mpq文件
	/************************************************************************/
	bool create(const char* file_name);

	/************************************************************************/
	/* 打开一个mpq文件
	/************************************************************************/
	bool open(const char* file_name);

	/************************************************************************/
	/* 备份一个mpq文件信息,file_name:备份文件名
	/************************************************************************/
	bool backup(const char* file_name);
	bool revert(const char* file_name,const char* file_revert);

	/************************************************************************/
	/* 关闭mpq文件
	/************************************************************************/
	void close(bool bSave = true);
	
	/************************************************************************/
	/* 从包中读取文件信息
	/************************************************************************/
	unsigned char* read_file(const char* file_name,uint32& size);
	unsigned char* read_node(MPQHashNode* pNode,uint32& size);
	unsigned char* read_block(MPQBlock* pBlock,uint32& size);

	/************************************************************************/
	/* 从包中追加文件信息
	/************************************************************************/
	bool append_file(const char* file_name,bool bCompress = false,bool bEncrypt = false);

	/************************************************************************/
	/* 从包中删除文件信息
	/************************************************************************/
	void remove_file(const char* file_name);

public:
	/************************************************************************/
	/* 向mpq中追加数据，返回块索引，失败返回 MPQ_BLOCK_INVALID           
	/************************************************************************/
	uint32	append_data(const uchar* pData,uint32 size,bool bCompress = false,bool bEncrypt = false);
	void	remove_data(MPQHashNode* pNode);
	void	remove_data(MPQBlock* pBlock);
	void	remove_data(uint32 idx);

	bool is_block_valid(uint32 idx);
	bool is_hash_node_valid(MPQHashNode* pNode);
	bool is_hash_node_valid(uint32 idx);
	bool is_hash_node_hashed(uint32 idx);

	void reset_hash_node(MPQHashNode* pNode);

	void read_header();
	void read_hash_table();
	void read_block_table();

	void write_header();
	void write_hash_table();
	void write_block_table();

	void clear_block_table();

	void log_block_table(const char* log);

protected:
	pthread_mutex_t m_readMutex;
	FILE*			m_fptr;
	MPQHeader		m_stHeader;
	MPQHashNode		m_hashTable[MPQ_HASH_TABLE_SIZE];
	typedef vector<MPQBlock*> MPQBlockTable;
	MPQBlockTable	m_blockTable;
	unsigned long	m_IdentCode;
};

#endif