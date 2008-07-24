/***************************************************************************
 * copyright            : (C) 2008 Daniel Jones <danielcjones@gmail.com> 
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy 
 * defined in Section 14 of version 3 of the license.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **************************************************************************/


#ifndef AMAROK_DYNAMICPLAYLIST_H
#define AMAROK_DYNAMICPLAYLIST_H

#include "Meta.h"

#include <QDomElement>
#include <QSharedData>


namespace Dynamic {

/**
 * Provides a basis for dynamic playlists which operate like a stream
 * of tracks, rather than a list.
 **/
class DynamicPlaylist : public QObject, public QSharedData
{
    Q_OBJECT

    public:
        DynamicPlaylist();
        DynamicPlaylist( Collection* );

        virtual ~DynamicPlaylist();

        virtual QDomElement xml() const;

        virtual Meta::TrackPtr  getTrack() = 0;
        virtual Meta::TrackList getTracks(int);

        QString title() const;
        int upcomingCount() const;
        int previousCount() const;

        void setTitle( QString );
        void setUpcomingCount( int );
        void setPreviousCount( int );

        virtual void requestAbort() {}

    public slots:
        virtual void recalculate();

    protected:
        Collection* m_collection;
        QString m_title;
        int     m_upcoming;
        int     m_previous;
};


typedef KSharedPtr<DynamicPlaylist> DynamicPlaylistPtr;
typedef QList<DynamicPlaylistPtr> DynamicPlaylistList;



}

Q_DECLARE_METATYPE( Dynamic::DynamicPlaylistPtr )
Q_DECLARE_METATYPE( Dynamic::DynamicPlaylistList )

#endif

