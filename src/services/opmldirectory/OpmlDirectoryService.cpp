/****************************************************************************************
 * Copyright (c) 2008 Nikolaj Hald Nielsen <nhn@kde.org>                                *
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

#include "OpmlDirectoryService.h"

#include "core/support/Debug.h"
#include "core/support/Components.h"
#include "core/interfaces/Logger.h"
#include "browsers/CollectionTreeItem.h"
#include "browsers/SingleCollectionTreeItemModel.h"
#include "OpmlDirectoryInfoParser.h"
#include "OpmlDirectoryModel.h"
#include "playlistmanager/PlaylistManager.h"
#include "core/podcasts/PodcastProvider.h"
#include "ServiceSqlRegistry.h"

#include <KTemporaryFile>
#include <threadweaver/ThreadWeaver.h>

#include <typeinfo>

using namespace Meta;

AMAROK_EXPORT_PLUGIN( OpmlDirectoryServiceFactory )

void OpmlDirectoryServiceFactory::init()
{
    ServiceBase* service = new OpmlDirectoryService( this, "OpmlDirectory", i18n( "Podcast Directory" ) );
    m_activeServices << service;
    m_initialized = true;
    emit newService( service );
}


QString OpmlDirectoryServiceFactory::name()
{
    return "OpmlDirectory";
}

KPluginInfo OpmlDirectoryServiceFactory::info()
{
    KPluginInfo pluginInfo( "amarok_service_opmldirectory.desktop", "services" );
    pluginInfo.setConfig( config() );
    return pluginInfo;
}


KConfigGroup OpmlDirectoryServiceFactory::config()
{
    return Amarok::config( "Service_OpmlDirectory" );
}


OpmlDirectoryService::OpmlDirectoryService( OpmlDirectoryServiceFactory* parent, const QString &name, const QString &prettyName )
 : ServiceBase( name, parent, true, prettyName )
{
    setShortDescription( i18n( "A large listing of podcasts" ) );
    setIcon( KIcon( "view-services-opml-amarok" ) );

    setLongDescription( i18n( "A comprehensive list of searchable podcasts that you can subscribe to directly from within Amarok." ) );

    KIconLoader loader;
    setImagePath( loader.iconPath( "view-services-opml-amarok", -128, true ) );
    
    m_serviceready = true;
    emit( ready() );
}


OpmlDirectoryService::~OpmlDirectoryService()
{
}

void OpmlDirectoryService::polish()
{
    generateWidgetInfo();
    if ( m_polished )
        return;

    //do not allow this content to get added to the playlist. At least not for now
    setPlayableTracks( false );

    KHBox * bottomPanelLayout = new KHBox;
    bottomPanelLayout->setParent( m_bottomPanel );

    m_subscribeButton = new QPushButton();
    m_subscribeButton->setParent( bottomPanelLayout );
    m_subscribeButton->setText( i18n( "Subscribe" ) );
    m_subscribeButton->setObjectName( "subscribeButton" );
    m_subscribeButton->setIcon( KIcon( "get-hot-new-stuff-amarok" ) );

    m_subscribeButton->setEnabled( false );

    connect( m_subscribeButton, SIGNAL( clicked() ), this, SLOT( subscribe() ) );

    setInfoParser( new OpmlDirectoryInfoParser() );

    Amarok::PrettyTreeView* view = new Amarok::PrettyTreeView( this );
    view->setHeaderHidden( true );
    view->setFrameShape( QFrame::NoFrame );
    view->setDragEnabled ( true );
    view->setSortingEnabled( false );
    view->setDragDropMode ( QAbstractItemView::DragOnly );
    setView( view );
    setModel( new OpmlDirectoryModel( KUrl( "file:///home/bart/tmp/amarok_podcasts.opml"), this ) );

    m_polished = true;
}

void OpmlDirectoryService::itemSelected( CollectionTreeItem * selectedItem )
{
    DEBUG_BLOCK
    return;
}

void OpmlDirectoryService::subscribe()
{
}

#include "OpmlDirectoryService.moc"
