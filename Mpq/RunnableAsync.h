#ifndef __RunnableAsync_h_
#define __RunnableAsync_h_

#include "pthread.h"

class IRunnable
{
public:
	IRunnable(){}
	virtual ~IRunnable(){}

	virtual void execute() = 0;
};

class CThread
{
public:
	CThread(IRunnable* pRunnable);
	~CThread();

public:
	void run();
	void init();
	void release();

private:
	static void* threadProc(void* param);

private:
	IRunnable* m_pRunnable;
};

#define doAsync(R) new CThread(R)

#endif