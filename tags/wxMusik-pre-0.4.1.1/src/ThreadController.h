/*
 *  ThreadController.h
 *
 *  This class is used for controlling a thread. Makes life easier.
 *  
 *  Copyright (c) 2003 Casey Langen (casey@bak.rr.com)
 *	Contributors: Simon Windmill, Dustin Carter, Gunnar Roth, Wade Brainerd
 *
 *  See the file "license.txt" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/
#ifndef THREADCONTROLLER_H
#define THREADCONTROLLER_H
class CThreadController
{
public:
	CThreadController()
	{
		m_pThread = NULL;
	}
	~CThreadController()
	{
		Empty();
	}
	void Attach(wxThread * thrd)
	{
		wxASSERT(!thrd->IsDetached()); // only non detached threads allowed
		Empty();
		m_pThread = thrd;
	}
	wxThreadError AttachAndRun(wxThread * thrd)
	{
		Attach(thrd);
		m_pThread->Create();
		return m_pThread->Run();
	}
	void Cancel()
	{
		Empty();
	}
	wxThread * Thread() { return m_pThread;}
	wxThread::ExitCode Join()
	{
		wxThread::ExitCode rc = 0;
		if(m_pThread)  // if m_pThread is null , we  have possibly called thread->Delete() ( see Empty() method)  so we dont delete the object right now. we are called recursive, because ->Delete() dispatches messages while waiting for thread ending
		{
			wxThread* ptmpThread = m_pThread;
			m_pThread = NULL;
			rc = ptmpThread->Wait(); // wait does also dispatch messages. now that m_pThread is null, the Cancel/Empty methods cannot interfere with us here.
			delete ptmpThread;
		}
		return rc;
	}

	bool IsAlive()
	{
		return m_pThread && m_pThread->IsAlive();
	}
protected:
	void Empty()
	{
		if(m_pThread)
		{
			wxThread* ptmpThread = m_pThread;
			m_pThread = NULL;	  // protect against deletion of the thread object in our Join() method, thread->Delete() does dispatch messages, if such a message leads to a call of Join() ,this would delete the thread object , => crash
			ptmpThread->Delete();
			delete ptmpThread;
			
		}
	}
private:
	wxThread *m_pThread;
};
#endif // THREADCONTROLLER_H
