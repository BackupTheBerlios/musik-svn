/*
*  Playlist.h
*  
*  Copyright (c) 2005 Gunnar Roth (gunnar.roth@gmx.de)
*	
*
*  See the file "license.txt" for information on usage and redistribution
*  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/
#ifndef PLFILE_H
#define PLFILE_H

#include <wx/fileconf.h>
#include <wx/url.h>
#include <wx/txtstrm.h>
#include <vector>

class MusikURL : public wxURL
{
public:
    MusikURL(const wxString& sUrl)
        : wxURL(sUrl)
    {
    }
    MusikURL(const wxURI& url)
        : wxURL(url)
    {
    }
    wxURL& MusikURL::operator = (const wxURI& url)
    {
        wxURL::operator = (url);
        return *this;
    }
    wxURL& MusikURL::operator = (const wxString& url)
    {
        wxURL::operator = (url);
        return *this;
    }
    wxInputStream *GetInputStream(int nTimeoutSecs)
    {
        if (m_protocol)
                m_protocol->SetTimeout(nTimeoutSecs);
        return wxURL::GetInputStream();
    }
};

struct PLFileEntry
{
    PLFileEntry():LengthSecs(0){}
    ~PLFileEntry(){}
    wxString File;
    wxString Artist;
    wxString Title;
    int LengthSecs;
};

class PLFile : public std::vector<PLFileEntry>
{
    wxString m_sProxy;
public:

    enum Type {Type_PLS,Type_M3U};
    PLFile(const wxString & sProxy = wxEmptyString)
        :m_sProxy(sProxy)
    {}
    
    bool Read(const wxString & name)
    {
        MusikURL url(name);
        return Read(url);
    }
    bool Read(MusikURL & url)
    {
        Type t;
        if(url.GetPath().Right(4).IsSameAs(wxT(".pls")))
            t = Type_PLS;
        else if(url.GetURL().Right(4).IsSameAs(wxT(".pls")))
            t = Type_PLS;
        else if(url.GetPath().Right(4).IsSameAs(wxT(".m3u")))
            t = Type_M3U;
        else if(url.GetURL().Right(4).IsSameAs(wxT(".m3u")))
            t = Type_M3U;
        else
            return false;
        if (url.GetError() != wxURL_NOERR)
            return false;
        if(!m_sProxy.IsEmpty() && url.GetScheme().IsSameAs(wxT("http")))
            url.SetProxy(m_sProxy);
        wxInputStream *in_stream = url.GetInputStream(10);
        if(!in_stream)
        {
            wxLogWarning(_("Cannot load data from %s."),url.GetURL().c_str());
            return false;
        }
        switch(t)
        {
        case Type_PLS:
            ReadPLS(*in_stream);
            break;
        case Type_M3U:
            ReadM3U(*in_stream);
            break;
        }
        delete in_stream;
        return true;
        
    }
protected:

    void ReadPLS(wxInputStream &in_stream)
    {
        wxFileConfig cfg(in_stream);
        cfg.SetPath(wxT("playlist"));
        int numentries = cfg.Read(wxT("NumberOfEntries"),0L);
        for(int i = 1; i <= numentries;i++)
        {
            PLFileEntry e;
            e.File = cfg.Read(wxString(wxT("File")) << i);
            e.Artist = cfg.Read(wxString(wxT("Artist")) << i);
            e.Title = cfg.Read(wxString(wxT("Title")) << i);
            cfg.Read(wxString(wxT("Length")) << i, &e.LengthSecs,0);
            if(!e.File.IsEmpty())
                push_back(e);
        }
    }
    void ReadM3U(wxInputStream &in_stream)
    {
       wxTextInputStream text_stream( in_stream );
       int numLine = 0;
       bool bExtm3u = false;
       PLFileEntry e;
       while(!in_stream.Eof())
       {
         numLine++;
         wxString l = text_stream.ReadLine();
         if((numLine ==1) && l.IsSameAs(wxT("#EXTM3U")))
            bExtm3u = true;
         l.Trim();
         if(l.IsEmpty())
             continue;
         if(l[0] == '#')
         { 
             if(bExtm3u)
             {
                 wxString rest;
                 if(l.StartsWith(wxT("#EXTINF:"),&rest))
                 {
                     int pos = -1;
                     StringToInt(rest,&e.LengthSecs);
                     const wxString &text = rest.AfterFirst(',');
                     pos = text.Find(wxT(" - "));
                     if(pos == -1)
                     {
                         e.Title = text;
                         e.Artist.Empty();
                     }
                     else
                     {
                         e.Artist = text.Left(pos);
                         e.Title = text.Right(text.Length() - (pos + 3));
                     }
                 }
             }
             continue;
         }
         e.File = l;
         if(!e.File.IsEmpty())
             push_back(e);
       }
    }

};
#endif
