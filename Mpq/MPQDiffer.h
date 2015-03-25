#ifndef __MPQDiffer_h_
#define __MPQDiffer_h_

#include "ProgressData.h"

class MPQDiffer : public CProgressData
{
public:
	MPQDiffer();
	~MPQDiffer();

public:
	bool compare(const char* mpq_old,const char* mpq_new,const char* mpq_diff);

	void onComplete();
	bool isBusy(){ return !m_bFree; }

protected:
	bool	m_bFree;
};

#endif