#include "MPQPatcher.h"
#include "MPQPackage.h"
#include "RunnableAsync.h"

class MPQPatcherAsync : public IRunnable
{
public:
	MPQPatcherAsync(MPQPatcher* pOwner,const char* mpq,const char* patch,const char* bak) : m_pOwner(pOwner){
		m_sMpqFile = mpq?mpq:"";
		m_sMpqPatchFile = patch?patch:"";
		m_sMpqBakFile = bak?bak:"";
	}
	~MPQPatcherAsync(){}

protected:
	unsigned long patch(const char* mpq,const char* mpq_patch,const char* bak)
	{
		if ( !mpq || !mpq_patch || !bak)
		{
			return 0;
		}

		unsigned long add_count = 0;
		unsigned long mod_count = 0;
		unsigned long del_count = 0;
		MPQPackage* ptr_mpq = NULL;
		MPQPackage* ptr_mpq_patch = NULL;

		// 打开日志文件
		FILE* fp_log = fopen("log.txt","w+");

		do 
		{
			ptr_mpq = new MPQPackage();
			ptr_mpq_patch = new MPQPackage();
			if ( !ptr_mpq || !ptr_mpq_patch )
			{
				if ( fp_log )
				{
					fprintf(fp_log,"new MPQPackage failed.\n");
				}
				break;
			}

			if ( !ptr_mpq->open(mpq) || !ptr_mpq_patch->open(mpq_patch) )
			{
				if ( fp_log )
				{
					fprintf(fp_log,"open mpq file failed.mpq:%s,mpq_patch:%s\n",mpq,mpq_patch);
				}
				break;
			}

			// 对原始mpq进行备份
			ptr_mpq->backup(bak);

			m_pOwner->setRange(MPQ_HASH_TABLE_SIZE);
			// 比对hash表
			for ( uint32 i = 0;i<MPQ_HASH_TABLE_SIZE; ++i )
			{
				m_pOwner->setPos(i);
				MPQHashNode* pNode = ptr_mpq->get_hash_node(i);
				MPQHashNode* pNodePatch = ptr_mpq_patch->get_hash_node(i);

				if ( ptr_mpq_patch->is_hash_node_valid(i) )	// 要增加或者修改的数据
				{
					MPQHashNode* pNodeValid = ptr_mpq->get_hash_node_valid(i,pNodePatch->verify_code_a,pNodePatch->verify_code_b);
					// 老包中有
					if ( pNodeValid )
					{
						MPQBlock* pBlock = ptr_mpq->get_block(pNodeValid);
						MPQBlock* pBlockPatch = ptr_mpq_patch->get_block(pNodePatch);

						string md5Code = MD5::binaryToHexString(pBlock->md5,MPQ_FILE_MD5_CODE_LEN);
						string md5CodePatch = MD5::binaryToHexString(pBlockPatch->md5,MPQ_FILE_MD5_CODE_LEN);

						if ( md5Code == md5CodePatch && pBlock->size == pBlockPatch->size)
						{
							if ( fp_log )
							{
								fprintf(fp_log,"ignored md5:%s,file: %s\n",md5Code.c_str(),pBlock->file_name.c_str());
							}
							continue;
						}
						else
						{
							// 删除老数据
							ptr_mpq->remove_data(pNodeValid);

							uint32 size = 0;
							unsigned char* pData = ptr_mpq_patch->read_block(pBlockPatch,size);

							pNodeValid->verify_code_a = pNodePatch->verify_code_a;
							pNodeValid->verify_code_b = pNodePatch->verify_code_b;
							pNodeValid->block_index = ptr_mpq->append_data(pData,size,pBlock->is_mark(MPQ_FILE_COMPRESS),pBlock->is_mark(MPQ_FILE_ENCRYPT));
							MPQBlock* pBlockNew = ptr_mpq->get_block(pNodeValid);
							memcpy(pBlockNew->md5,pBlockPatch->md5,MPQ_FILE_MD5_CODE_LEN);
							pBlockNew->file_name = pBlockPatch->file_name;

							if ( fp_log )
							{
								fprintf(fp_log,"mod md5:%s,file: %s\n",MD5::binaryToHexString(pBlockNew->md5,MPQ_FILE_MD5_CODE_LEN).c_str(),pBlockNew->file_name.c_str());
							}

							++mod_count;

							if ( pData )
							{
								delete[] pData;
							}
						}
					}
					else
					{
						MPQBlock* pBlockPatch = ptr_mpq_patch->get_block(pNodePatch);

						uint32 size = 0;
						unsigned char* pData = ptr_mpq_patch->read_block(pBlockPatch,size);

						// 加入新的数据
						MPQHashNode* pNodeNew = ptr_mpq->get_hash_node_new(i,pNodePatch->verify_code_a,pNodePatch->verify_code_b);
						if ( pNodeNew )
						{
							pNodeNew->block_index = ptr_mpq->append_data(pData,size,pBlockPatch->is_mark(MPQ_FILE_COMPRESS),pBlockPatch->is_mark(MPQ_FILE_ENCRYPT));
							MPQBlock* pBlockNew = ptr_mpq->get_block(pNodeNew);
							memcpy(pBlockNew->md5,pBlockPatch->md5,MPQ_FILE_MD5_CODE_LEN);
							pBlockNew->file_name = pBlockPatch->file_name;

							if ( fp_log )
							{
								fprintf(fp_log,"add md5:%s,file: %s\n",MD5::binaryToHexString(pBlockNew->md5,MPQ_FILE_MD5_CODE_LEN).c_str(),pBlockNew->file_name.c_str());
							}

							++add_count;
						}
						else
						{
							if ( fp_log )
							{
								fprintf(fp_log,"no free hash node found!\n");
							}
						}

						if ( pData )
						{
							delete pData;
						}
					}
				}
				else if ( ptr_mpq_patch->is_hash_node_hashed(i) ) // 要删除的数据
				{
					MPQHashNode* pNodeDelete = ptr_mpq->get_hash_node_valid(i,pNodePatch->verify_code_a,pNodePatch->verify_code_b);
					if ( pNodeDelete )
					{
						MPQBlock* pBlockDelete = ptr_mpq->get_block(pNodeDelete);
						ptr_mpq->remove_data(pNodeDelete);
						if ( fp_log )
						{
							fprintf(fp_log,"del md5:%s,file: %s\n",MD5::binaryToHexString(pBlockDelete->md5,MPQ_FILE_MD5_CODE_LEN).c_str(),pBlockDelete->file_name.c_str());
						}
						++del_count;
						
					}
				}
			}
		} while (0);

		if ( fp_log )
		{
			fprintf(fp_log,"added:%u,modified:%u,deleted:%u\n",add_count,mod_count,del_count);
			fclose(fp_log);
		}

		if ( ptr_mpq_patch )
		{
			ptr_mpq_patch->close();
			delete ptr_mpq_patch;
		}
		
		if ( ptr_mpq )
		{
			ptr_mpq->log_block_table("patch.log");
			ptr_mpq->close();
			delete ptr_mpq;
		}
		
		return add_count+mod_count+del_count;
	}

	void execute(){
		if ( m_sMpqFile.empty() || m_sMpqPatchFile.empty())
		{
			return;
		}

		patch(m_sMpqFile.c_str(),m_sMpqPatchFile.c_str(),m_sMpqBakFile.c_str());

		m_pOwner->onComplete();
	}

	MPQPatcher* m_pOwner;
	string m_sMpqFile;
	string m_sMpqPatchFile;
	string m_sMpqBakFile;
};


MPQPatcher::MPQPatcher() : m_bFree(true)
{

}

MPQPatcher::~MPQPatcher()
{

}

bool MPQPatcher::patch(const char* mpq_src,const char* mpq_patch,const char* mpq_bak)
{
	if ( isBusy() )
	{
		return false;
	}
	reset();
	m_bFree = false;
	doAsync(new MPQPatcherAsync(this,mpq_src,mpq_patch,mpq_bak));

	return true;
}

void MPQPatcher::onComplete()
{
	m_bFree = true;
}