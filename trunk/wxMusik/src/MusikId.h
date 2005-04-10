#ifndef MUSIKID_H
#define MUSIKID_H

#include "MusikUtils.h"

#include <memory> // for auto_ptr

class MusikSongId
{
public:
    explicit MusikSongId(int id)
        :m_Id(id)
        ,m_pSong(NULL)
    {
        Check1 = 0;
        bChosenByUser = 1;
        bForcePlay = 0;
    }
    explicit MusikSongId()
        :m_Id(ExternalSong)
        ,m_pSong(NULL)
    {
        Check1 = 0;
        bChosenByUser = 1;
        bForcePlay = 0;
    }
    explicit MusikSongId(CMusikSong* pSong)
        :m_Id(ExternalSong)
        ,m_pSong(pSong)
    {}
    ~MusikSongId();
    MusikSongId( const MusikSongId & id)
    {
        if(id.m_pSong)
        {
            m_pSong = new CMusikSong(*id.m_pSong);
        }
        else
           m_pSong = NULL;
        m_Id = id.m_Id;
    }
    MusikSongId & operator=(const MusikSongId &id)
    {
        if(this != &id)
        {
            if(m_pSong)
            {
                delete m_pSong;
            }
            if(id.m_pSong)
                m_pSong = new CMusikSong(*id.m_pSong);
            else
                m_pSong = NULL;
            m_Id = id.m_Id;
           
        }
        return *this;
    }
    bool IsFormat(EMUSIK_FORMAT_TYPE ft) const
    {
        if(m_pSong)
        {
            return m_pSong->MetaData.eFormat == ft;
        }
        if( ft == MUSIK_FORMAT_NETSTREAM)
            return false;// HACK: for a MUSIK_FORMAT_NETSTREAM  m_pSong is always non-null.
        std::auto_ptr<CMusikSong> pSong = Song();
        return pSong->MetaData.eFormat == ft;
    }
    bool IsInLibrary()
    {
        return m_Id >= 0;
    }
    operator int() const
    {
        return m_Id;
    }
    int Id() const
    {
        return m_Id;
    }
    bool operator ==( const MusikSongId &id) const
    {
        if(m_Id > 0)
            return m_Id == id.m_Id;
        if(m_pSong && id.m_pSong)
        {
            return m_pSong->MetaData.Filename == id.m_pSong->MetaData.Filename;
        }
        return false;
    }
    bool operator !=( const MusikSongId &id) const
    {
        return !operator==(id);
    }
    std::auto_ptr<CMusikSong> Song() const;

    CMusikSong& SongRef()
    {
        if(m_pSong ==  NULL)
        {
            if(m_Id >= 0)
            {
                m_pSong = Song().release();
            }
            else
                m_pSong = new CMusikSong;
        }
        return *m_pSong;
    }

 private:
    enum {ExternalSong = -1};

    int m_Id;
    CMusikSong *m_pSong;
public:
    unsigned int			Check1:1;		//--- for tag dlg stuff, checks to see if it needs to be written to file / db ---//
    unsigned int			bChosenByUser:1;
    unsigned int			bForcePlay:1;
};
#endif