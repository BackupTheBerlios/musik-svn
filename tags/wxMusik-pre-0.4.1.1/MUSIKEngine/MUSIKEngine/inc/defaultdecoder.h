#ifndef MUSIKDEFAULTDECODER_H
#define MUSIKDEFAULTDECODER_H
#include "MUSIKEngine/inc/decoder.h"
#include "MUSIKEngine/inc/imusikstreamout.h"
class   MUSIKDefaultDecoder: public MUSIKDecoder
{
public:
	MUSIKDefaultDecoder(IMUSIKStreamOutDefault * pIMUSIKStreamOutDefault);
	virtual bool Start();
	virtual bool CanSeek()
	{
		return m_pIMUSIKStreamOutDefault->CanSeek();
	}
	virtual int GetTime();
	virtual bool SetTime( int nTimeMS);
	virtual int GetLengthMs();
	bool Close();
	virtual const char * Type()
	{
		return m_pIMUSIKStreamOutDefault->Type();
	}
protected:
	virtual bool OpenMedia(const char *FileName);
	virtual int DecodeBlocks(unsigned char *buff,int len)
	{
		return false;
	}
	virtual bool DoSeek(int)
	{
		return false;
	}

	IMUSIKStreamOutDefault * m_pIMUSIKStreamOutDefault;
private:

};
#endif
