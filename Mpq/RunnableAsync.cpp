#include "RunnableAsync.h"

CThread::CThread(IRunnable* pRunnable) : m_pRunnable(pRunnable) 
{
	init();
}

CThread::~CThread()
{

}

void CThread::run()
{
	if ( m_pRunnable )
	{
		m_pRunnable->execute();
	}
}

void CThread::init()
{
	if ( m_pRunnable )
	{
		pthread_attr_t attr;
		pthread_attr_init(&attr);

		struct sched_param stShedParam;
		pthread_attr_getschedparam(&attr, &stShedParam);

		stShedParam.sched_priority = sched_get_priority_min(SCHED_OTHER);
		pthread_attr_setschedparam(&attr, &stShedParam);

		pthread_t hThread;
		pthread_create(&hThread, &attr, &CThread::threadProc, this);
	}
}

void CThread::release()
{
	if ( m_pRunnable )
	{
		delete m_pRunnable;
	}

	delete this;
}

void* CThread::threadProc(void* param)
{
	CThread* pThread = (CThread*)(param);

	pThread->run();
	pThread->release();

	return 0;
}