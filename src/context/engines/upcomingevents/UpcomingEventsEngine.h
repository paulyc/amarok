/****************************************************************************************
 * Copyright (c) 2009 Oleksandr Khayrullin <saniokh@gmail.com>                          *
 * Copyright (c) 2009 Nathan Sala <sala.nathan@gmail.com>                               *
 * Copyright (c) 2009-2010 Ludovic Deveaux <deveaux.ludovic31@gmail.com>                *
 *                                                                                      *
 * This program is free software; you can redistribute it and/or modify it under        *
 * the terms of the GNU General Public License as published by the Free Software        *
 * Foundation; either version 2 of the License, or (at your option) any later           *
 * version.                                                                             *
 *                                                                                      *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY      *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A      *
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.             *
 *                                                                                      *
 * You should have received a copy of the GNU General Public License along with         *
 * this program.  If not, see <http://www.gnu.org/licenses/>.                           *
 ****************************************************************************************/

#ifndef AMAROK_UPCOMINGEVENTS_ENGINE
#define AMAROK_UPCOMINGEVENTS_ENGINE

#include "context/applets/upcomingevents/LastFmEvent.h"
#include "context/DataEngine.h"
#include "core/meta/Meta.h"
#include "core/engine/EngineObserver.h"
#include "network/NetworkAccessManagerProxy.h"

// Qt
#include <QDomDocument>
#include <QLocale>
#include <QXmlStreamReader>

class QNetworkReply;

using namespace Context;

/**
 * \class UpcomingEventsEngine
 *
 * This class provide UpcomingEvents data for use in Context applets
 */
class UpcomingEventsEngine : public DataEngine, public Meta::Observer, public Engine::EngineObserver
{
    Q_OBJECT

public:
    /**
     * \brief Constructor
     *
     * Creates a new instance of UpcomingEventsEngine
     */
    UpcomingEventsEngine( QObject* parent, const QList<QVariant>& args );

    /**
     * \brief Destructor
     *
     * Destroys an UpcomingEventsEngine instance
     */
    virtual ~UpcomingEventsEngine();

    /**
     * This method is called when the metadata of a track has changed.
     * The called class may not cache the pointer
     */
    using Observer::metadataChanged;
    void metadataChanged( Meta::TrackPtr track );

    /**
     * Reimplemented from Engine::EngineObserver
     */
    void engineNewTrackPlaying();

protected:
    /**
     * Reimplemented from Plasma::DataEngine
     */
    bool sourceRequestEvent( const QString &name );

private:
    /**
     * Sends the data to the observers (e.g UpcomingEventsApplet)
     */
    void updateDataForArtist();

    /**
     * Get events for specific venues
     * @param ids LastFm's venue ids
     */
    void updateDataForVenues( QList<int> ids );

    /**
     * filterEvents filters a list of events depending on settings
     * @param events a list of events to filter
     * @return a new list of events that satisfies filter settings
     */
    LastFmEvent::List filterEvents( const LastFmEvent::List &events ) const;

    /**
     * The value can be "AllEvents", "ThisWeek", "ThisMonth" or "ThisYear"
     */
    QString m_timeSpan;

    /**
     * The current track playing
     */
    Meta::TrackPtr m_currentTrack;

    /**
     * Current URLs of events being fetched
     */
    QSet<KUrl> m_urls;

private slots:
    void artistEventsFetched( const KUrl &url, QByteArray data, NetworkAccessManagerProxy::Error e );
    void venueEventsFetched( const KUrl &url, QByteArray data, NetworkAccessManagerProxy::Error e );
};

#endif
