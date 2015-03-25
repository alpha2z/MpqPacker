#ifndef __MPQPatcher_h_
#define __MPQPatcher_h_

#include "ProgressData.h"

class MPQPatcher : public CProgressData
{
public:
	MPQPatcher();
	~MPQPatcher();

public:
	bool patch(const char* mpq_src,const char* mpq_patch,const char* mpq_bak);

	void onComplete();
	bool isBusy(){ return !m_bFree; }
protected:
	bool	m_bFree;
};

#endif