#ifndef __ProgressData_h_
#define __ProgressData_h_

#include <string>
using namespace std;

class CProgressData
{
public:
	CProgressData() : m_nRange(0),m_nPos(0),m_sInfo(""){}
	virtual ~CProgressData(){}

	virtual void setRange(int range){ m_nRange = range; }
	virtual void setPos(int pos){ m_nPos = pos; }
	virtual void setInfo(const string& info){ m_sInfo = info; }

	virtual int		getRange(){ return m_nRange; }
	virtual int		getPos(){ return m_nPos; }
	virtual string	getInfo(){ return m_sInfo; }

	virtual void reset(){ m_nRange=0;m_nPos=0;m_sInfo = "";}
	
protected:
	int		m_nRange;
	int		m_nPos;
	string	m_sInfo;
};

#endif