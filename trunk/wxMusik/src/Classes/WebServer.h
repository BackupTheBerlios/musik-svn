/*
 *  WebServer.h
 *
 *  Musik's internal webserver. 
 *  Someday this will be quite the little jewel.
 *  
 *  Copyright (c) 2003 Casey Langen (casey@bak.rr.com)
 *	Contributors: Simon Windmill, Dustin Carter, Gunnar Roth, Wade Brainerd
 *
 *  See the file "license.txt" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/

#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "wx/socket.h"

#define MUSIK_SERVER_ID 2000

#define MAX_BUF_LEN 1024

class CMusikPlayer;


class CMusikWebServer : public wxEvtHandler
{
public:
	CMusikWebServer(CMusikPlayer * pPlayer);
	~CMusikWebServer();

public:
	void Start(int Port);
	void Stop();
	bool IsRunning() { return m_bRunning; };
    int Port() {return m_nPort;}

	DECLARE_EVENT_TABLE()
private:
    class HttpResponse
    {
    public: 
        HttpResponse(){}
        void Send(wxSocketBase*pSocket);
        void SetRC(const wxString & rc);
        void AddDataLine(const wxString & data);
        void AddHeader(const wxString & Header);
        bool Ok(){return !m_sRC.IsEmpty();}
    private:
        wxString m_sRC;
        wxString m_sHeaders;
        wxString m_sData;
    };


	void OnServerEvent(wxSocketEvent& event);
	void OnSocketEvent(wxSocketEvent& event);
	
	
	void  ProcessRequest(const wxString &reqstr,HttpResponse &hr);
	int ReadLine(wxSocketBase*pSocket,wxString& outstr);
	void WriteLine(wxSocketBase*pSocket,const wxString &str);

	bool m_bRunning;
    int m_nPort;
	wxSocketServer *pServer;

	CMusikPlayer * m_pPlayer;
};

#endif
