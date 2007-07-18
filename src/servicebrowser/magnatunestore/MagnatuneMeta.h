/***************************************************************************
 *   Copyright (c) 2007  Nikolaj Hald Nielsen <nhnFreespirit@gmail.com>    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02111-1307, USA.          *
 ***************************************************************************/

#ifndef JAMENDOTYPES_H
#define JAMENDOTYPES_H


#include "../servicemetabase.h"

#include <QDateTime>
#include <QString>
#include <QStringList>
#include <QList>


class MagnatuneMetaFactory : public ServiceMetaFactory
{

public:
    MagnatuneMetaFactory( const QString &dbPrefix );
    virtual ~MagnatuneMetaFactory() {}

    virtual int getTrackSqlRowCount();
    virtual QString getTrackSqlRows();
    virtual TrackPtr createTrack( const QStringList &rows );

    virtual int getAlbumSqlRowCount();
    virtual QString getAlbumSqlRows();
    virtual AlbumPtr createAlbum( const QStringList &rows );

    virtual int getArtistSqlRowCount();
    virtual QString getArtistSqlRows();
    virtual ArtistPtr createArtist( const QStringList &rows );

    //virtual int getGenreSqlRowCount();
    //virtual QString getGenreSqlRows();
    virtual GenrePtr createGenre( const QStringList &rows );

};




class MagnatuneTrack  : public ServiceTrack
{


public:
    MagnatuneTrack( const QString &name );
    MagnatuneTrack( const QStringList &resultRow );

    QString lofiUrl();
    void setLofiUrl( const QString &url );

private:
    QString m_lofiUrl;

};


class MagnatuneArtist : public ServiceArtist
{

private:

    QString m_photoUrl;
    QString m_magnatuneUrl;

public:
    MagnatuneArtist( const QString &name );
    MagnatuneArtist( const QStringList &resultRow );

    void setPhotoUrl( const QString &photoUrl );
    QString photoUrl() const;

    void setMagnatuneUrl( const QString &url );
    QString magnatuneUrl() const;
};


class MagnatuneAlbum  : public ServiceAlbum
{
private:
    QString m_coverUrl;
    int m_launchYear;
    QString m_albumCode;

public:
    MagnatuneAlbum( const QString &name );
    MagnatuneAlbum( const QStringList &resultRow );

    void setCoverUrl( const QString &coverUrl );
    QString coverUrl() const;

    void setLaunchYear( int launchYear );
    int launchYear() const;

    void setAlbumCode(  const QString &albumCode );
    QString albumCode();

};

class MagnatuneGenre  : public ServiceGenre
{

public:
    MagnatuneGenre( const QString &name );
    MagnatuneGenre( const QStringList &resultRow );

};



#endif
