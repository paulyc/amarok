/*
   Copyright (C) 2008 Alejandro Wainzinger <aikawarazuni@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
*/

#ifndef MEDIADEVICECOLLECTION_H
#define MEDIADEVICECOLLECTION_H

#include "ConnectionAssistant.h"
#include "MediaDeviceHandler.h"
#include "MediaDeviceCollectionLocation.h"

#include "Collection.h"
#include "MemoryCollection.h"

#include "mediadevicecollection_export.h"

#include <KIcon>

#include <QtGlobal>

class MediaDeviceCollection;


/** HACK: Base and Factory are separate because Q_OBJECT does not work directly with templates.
Templates used to reduce duplicated code in subclasses.
*/


class MEDIADEVICECOLLECTION_EXPORT MediaDeviceCollectionFactoryBase : public Amarok::CollectionFactory
{
    Q_OBJECT

       public:
        virtual ~MediaDeviceCollectionFactoryBase();
        virtual void init();
    protected:
        MediaDeviceCollectionFactoryBase( ConnectionAssistant* assistant );

    public slots:
        // convenience slot
        void removeDevice( const QString &udi ) { deviceRemoved( udi ); }

    protected slots:
        virtual void deviceDetected( MediaDeviceInfo* info ); // detected type of device, connect it

    private slots:
                void deviceRemoved( const QString &udi );

    private:

        virtual Amarok::Collection* createCollection( MediaDeviceInfo* info ) = 0;

        ConnectionAssistant* m_assistant;
        QMap<QString, Amarok::Collection*> m_collectionMap;
};

template <class CollType>
class MEDIADEVICECOLLECTION_EXPORT MediaDeviceCollectionFactory : public MediaDeviceCollectionFactoryBase
{
    protected:
        MediaDeviceCollectionFactory( ConnectionAssistant *assistant )
        : MediaDeviceCollectionFactoryBase( assistant ) {}
        virtual ~MediaDeviceCollectionFactory() {}
    private:
        virtual Amarok::Collection* createCollection( MediaDeviceInfo* info )
        {
            return new CollType( info );
        }


};


class MEDIADEVICECOLLECTION_EXPORT MediaDeviceCollection : public Amarok::Collection, public MemoryCollection
{
    Q_OBJECT
    public:

        /** Collection-related methods */

        virtual ~MediaDeviceCollection();

        /**

        url-based methods can be abstracted via use of Amarok URLs
        subclasses simply define a protocol prefix, e.g. ipod

        */
        virtual bool possiblyContainsTrack( const KUrl &url ) const { Q_UNUSED(url); return false;} // TODO: NYI
        virtual Meta::TrackPtr trackForUrl( const KUrl &url ) { Q_UNUSED(url); return Meta::TrackPtr();  } // TODO: NYI

        virtual QueryMaker* queryMaker();
        virtual void startFullScan(); // TODO: this will replace connectDevice() call to parsetracks in handler

        // NOTE: incrementalscan and stopscan not implemented, might be used by UMS later though

        /**
            The protocol of uids coming from this collection.
            @return A string of the protocol, without the ://

            This has to be overridden for every device type, e.g. ipod://
        */
        virtual QString uidUrlProtocol() const { return QString(); } // TODO: NYI
        virtual QString collectionId() const; // uses udi

        virtual QString prettyName() const = 0; // NOTE: must be overridden based on device type
        virtual KIcon icon() const = 0; // NOTE: must be overridden based on device type

        virtual CollectionLocation* location() const { return new MediaDeviceCollectionLocation(this); } // NOTE: location will have same method calls always, no need to redo each time

        /** Capability-related methods */

        //virtual bool hasCapabilityInterface( Meta::Capability::Type type ) const;
        //virtual Meta::Capability* asCapabilityInterface( Meta::Capability::Type type );

        /** MediaDeviceCollection methods */

        //void parseDevice();

        //void deviceRemoved() { emit remove(); }

        QString udi() const { return m_udi; }

        //MediaDevice::MediaDeviceHandler* handler() { return m_handler; }

        //void updateTags( Meta::MediaDeviceTrack *track);
        //void writeDatabase(); // threaded

        /** MediaDeviceCollection-specific */

    public:
        MediaDeviceHandler* handler();

        

    signals:
        void collectionReady();
        void collectionDisconnected( const QString &udi );

        void copyTracksCompleted( bool success );

    public slots:
        // NOTE: must be overridden.  Parses tracks on successful handler connection.

        void connectDevice() {}

        // TODO: these two could be merged somehow
        //void disconnectDevice();
        //void slotDisconnect();

    protected:
        MediaDeviceCollection();

        QString                          m_udi;
        MediaDeviceHandler *m_handler;

    private:

};


#endif
