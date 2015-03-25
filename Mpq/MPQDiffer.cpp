#include "MPQDiffer.h"
#include "MPQPackage.h"
#include "RunnableAsync.h"

class MPQDifferAsync : public IRunnable
{
public:
	MPQDifferAsync(MPQDiffer* pOwner,const char* mpq_old,const char* mpq_new,const char* mpq_diff) : m_pOwner(pOwner){
		m_sMpqFileOld = mpq_old?mpq_old:"";
		m_sMpqFileNew = mpq_new?mpq_new:"";
		m_sMpqFileDiff = mpq_diff?mpq_diff:"";
		m_sErrorMsg = "";
	}
	~MPQDifferAsync(){}

protected:
	unsigned long compare(const char* mpq_old,const char* mpq_new,const char* mpq_diff)
	{
		if ( !mpq_old || !mpq_new || !mpq_diff )
		{
			return 0;
		}

		MPQPackage* ptr_pkg_old = NULL;
		MPQPackage* ptr_pkg_new = NULL;
		MPQPackage* ptr_pkg_diff = NULL;
		unsigned long add_count = 0;
		unsigned long mod_count = 0;
		unsigned long del_count = 0;

		m_pOwner->setRange(MPQ_HASH_TABLE_SIZE);

		// 打开日志文件
		FILE* fp_log = fopen("log.txt","w+");

		do 
		{
			ptr_pkg_old = new MPQPackage();
			ptr_pkg_new = new MPQPackage();
			ptr_pkg_diff = new MPQPackage();
			
			if ( !ptr_pkg_old || !ptr_pkg_new || !ptr_pkg_diff )
			{
				break;
			}

			if ( !ptr_pkg_old->open(mpq_old) || !ptr_pkg_new->open(mpq_new) || !ptr_pkg_diff->create(mpq_diff) )
			{
				break;
			}

			// 比对hash表
			for ( uint32 i = 0;i<MPQ_HASH_TABLE_SIZE; ++i )
			{
				m_pOwner->setPos(i);
				MPQHashNode* pNodeNew = ptr_pkg_new->get_hash_node(i);
				MPQHashNode* pNodeOld = ptr_pkg_old->get_hash_node(i);

				if ( ptr_pkg_new->is_hash_node_valid(pNodeNew) )
				{
					MPQHashNode* pNodeValidOld = ptr_pkg_old->get_hash_node_valid(i,pNodeNew->verify_code_a,pNodeNew->verify_code_b);
					// 老包中有这个文件
					if ( pNodeValidOld ) 
					{
						MPQBlock* pBlockNew = ptr_pkg_new->get_block(pNodeNew);
						MPQBlock* pBlockOld = ptr_pkg_old->get_block(pNodeValidOld);

						string md5CodeNew = MD5::binaryToHexString(pBlockNew->md5,MPQ_FILE_MD5_CODE_LEN);
						string md5CodeOld = MD5::binaryToHexString(pBlockOld->md5,MPQ_FILE_MD5_CODE_LEN);

						// 文件内容一样的
						if ( md5CodeNew == md5CodeOld && pBlockNew->size == pBlockOld->size )
						{
							
						}
						else
						{
							uint32 nSizeNew = 0;
							unsigned char* pDataNew = ptr_pkg_new->read_block(pBlockNew,nSizeNew);

							MPQHashNode* pNodeDiff = ptr_pkg_diff->get_hash_node_new_for_diff(i,pNodeNew->verify_code_a,pNodeNew->verify_code_b);

							uint32 blockIndex = ptr_pkg_diff->append_data(pDataNew,nSizeNew,pBlockNew->is_mark(MPQ_FILE_COMPRESS),pBlockNew->is_mark(MPQ_FILE_ENCRYPT));
							if ( blockIndex == MPQ_BLOCK_INVALID )
							{
								ptr_pkg_diff->reset_hash_node(pNodeDiff);
							}

							delete pDataNew;

							pNodeDiff->block_index = blockIndex;
							MPQBlock* pBlockDiff = ptr_pkg_diff->get_block(pNodeDiff);
							memcpy(pBlockDiff->md5,pBlockNew->md5,MPQ_FILE_MD5_CODE_LEN);
							pBlockDiff->file_name = pBlockNew->file_name;

							if ( fp_log )
							{
								fprintf(fp_log,"mod md5:%s,file: %s\n",MD5::binaryToHexString(pBlockDiff->md5,MPQ_FILE_MD5_CODE_LEN).c_str(),pBlockDiff->file_name.c_str());
							}

							++mod_count;
						}
					}
					// 老包中没有这个文件
					else
					{
						MPQBlock* pBlockNew = ptr_pkg_new->get_block(pNodeNew);

						uint32 nSizeNew = 0;
						unsigned char* pDataNew = ptr_pkg_new->read_block(pBlockNew,nSizeNew);

						MPQHashNode* pNodeDiff = ptr_pkg_diff->get_hash_node_new_for_diff(i,pNodeNew->verify_code_a,pNodeNew->verify_code_b);

						uint32 blockIndex = ptr_pkg_diff->append_data(pDataNew,nSizeNew,pBlockNew->is_mark(MPQ_FILE_COMPRESS),pBlockNew->is_mark(MPQ_FILE_ENCRYPT));
						if ( blockIndex == MPQ_BLOCK_INVALID )
						{
							ptr_pkg_diff->reset_hash_node(pNodeDiff);
						}
						delete pDataNew;

						pNodeDiff->block_index = blockIndex;
						MPQBlock* pBlockDiff = ptr_pkg_diff->get_block(pNodeDiff);
						memcpy(pBlockDiff->md5,pBlockNew->md5,MPQ_FILE_MD5_CODE_LEN);
						pBlockDiff->file_name = pBlockNew->file_name;

						if ( fp_log )
						{
							fprintf(fp_log,"add md5:%s,file: %s\n",MD5::binaryToHexString(pBlockDiff->md5,MPQ_FILE_MD5_CODE_LEN).c_str(),pBlockDiff->file_name.c_str());
						}

						++add_count;
					}
				}

				// 检测老包
				if ( ptr_pkg_old->is_hash_node_valid(pNodeOld) )
				{
					MPQHashNode* pNodeValidNew = ptr_pkg_new->get_hash_node_valid(i,pNodeOld->verify_code_a,pNodeOld->verify_code_b);
					// 新包中有
					if ( pNodeValidNew )
					{
						continue;
					}
					// 新包中没有
					else
					{
						MPQBlock* pBlockDelete = ptr_pkg_old->get_block(pNodeOld);
						MPQHashNode* pNodeDiff = ptr_pkg_diff->get_hash_node_new_for_diff(i,pNodeOld->verify_code_a,pNodeOld->verify_code_b);
						pNodeDiff->block_index = MPQ_BLOCK_INVALID;
						// deleted
						if ( fp_log )
						{
							fprintf(fp_log,"del md5:%s,file: %s\n",MD5::binaryToHexString(pBlockDelete->md5,MPQ_FILE_MD5_CODE_LEN).c_str(),pBlockDelete->file_name.c_str());
						}

						++del_count;
					}
				}
			}

			m_pOwner->setPos(MPQ_HASH_TABLE_SIZE);
		} while (0);

		if ( fp_log )
		{
			fprintf(fp_log,"added:%u,modified:%u,deleted:%u\n",add_count,mod_count,del_count);
			fclose(fp_log);
		}

		if ( ptr_pkg_diff )
		{
			ptr_pkg_diff->log_block_table("compare_diff.log");
			ptr_pkg_diff->close();
			delete ptr_pkg_diff;
		}

		if ( ptr_pkg_old )
		{
			ptr_pkg_old->close();
			delete ptr_pkg_old;
		}

		if ( ptr_pkg_new )
		{
			ptr_pkg_new->close();
			delete ptr_pkg_new;
		}

		return add_count+mod_count+del_count;
	}

	void execute(){
		if ( m_sMpqFileOld.empty() || m_sMpqFileNew.empty() || m_sMpqFileDiff.empty())
		{
			return ;
		}

		compare(m_sMpqFileOld.c_str(),m_sMpqFileNew.c_str(),m_sMpqFileDiff.c_str());

		m_pOwner->onComplete();
	}

	MPQDiffer* m_pOwner;
	string m_sMpqFileOld;
	string m_sMpqFileNew;
	string m_sMpqFileDiff;
	string m_sErrorMsg;
};

MPQDiffer::MPQDiffer() : m_bFree(true)
{

}

MPQDiffer::~MPQDiffer()
{

}

bool MPQDiffer::compare(const char* mpq_old,const char* mpq_new,const char* mpq_diff)
{
	if ( isBusy() )
	{
		return false;
	}
	reset();
	doAsync(new MPQDifferAsync(this,mpq_old,mpq_new,mpq_diff));
	m_bFree = false;

	return true;
}

void MPQDiffer::onComplete()
{
	m_bFree = true;
}