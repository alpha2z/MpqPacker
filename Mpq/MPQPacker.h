#ifndef __MPQPacker_h_
#define __MPQPacker_h_

#include "ProgressData.h"

class MPQPacker : public CProgressData
{
public:
	MPQPacker();
	~MPQPacker();

public:
	bool pack(const char* path,const char* mpq);
	bool packWithConfig(const char* path,const char* config,const char* mpq);

	void onComplete();
	bool isBusy(){ return !m_bFree; }

protected:
	bool	m_bFree;
};

#endif