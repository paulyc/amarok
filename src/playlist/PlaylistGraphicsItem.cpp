/***************************************************************************
 * copyright            : (C) 2007 Ian Monroe <ian@monroe.nu>              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License version 2        *
 *   as published by the Free Software Foundation.                         *
 ***************************************************************************/

#include "debug.h"
#include "meta/MetaUtility.h"
#include "AmarokMimeData.h"
#include "PlaylistGraphicsItem.h"
#include "PlaylistGraphicsView.h"
#include "PlaylistDropVis.h"
#include "PlaylistModel.h"
#include "PlaylistTextItem.h"
#include "TheInstances.h"

#include "KStandardDirs"

#include <QBrush>
#include <QDrag>
#include <QFontMetricsF>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QMimeData>
#include <QPen>
#include <QPixmapCache>
#include <QRadialGradient>
#include <QScrollBar>
#include <QStyleOptionGraphicsItem>

#include <KLocale>

struct Playlist::GraphicsItem::ActiveItems
{
    ActiveItems()
    : foreground( 0 )
    , bottomLeftText( 0 )
    , bottomRightText( 0 )
    , topLeftText( 0 )
    , topRightText( 0 )
    , lastWidth( -5 )
    , groupedTracks ( 0 )
     { }
     ~ActiveItems()
     {
        delete bottomLeftText;
        delete bottomRightText;
         delete foreground;
        delete topLeftText;
        delete topRightText;

      }

     QGraphicsRectItem* foreground;
     Playlist::TextItem* bottomLeftText;
     Playlist::TextItem* bottomRightText;
     Playlist::TextItem* topLeftText;
     Playlist::TextItem* topRightText;

     QColor overlayGradientStart;
     QColor overlayGradientEnd;

     int lastWidth;
     int groupedTracks;

     QRectF preDragLocation;
     Meta::TrackPtr track;
 };


const qreal Playlist::GraphicsItem::ALBUM_WIDTH = 50.0;
const qreal Playlist::GraphicsItem::MARGIN = 4.0;
QFontMetricsF* Playlist::GraphicsItem::s_fm = 0;
QSvgRenderer * Playlist::GraphicsItem::s_svgRenderer = 0;

Playlist::GraphicsItem::GraphicsItem()
    : QGraphicsItem()
    , m_items( 0 )
    , m_height( -1 )
    , m_groupMode( -1 )
    , m_groupModeChanged ( false )
{
    setZValue( 1.0 );
    if( !s_fm )
    {
        s_fm = new QFontMetricsF( QFont() );
        m_height =  qMax( ALBUM_WIDTH, s_fm->height() * 2 ) + 2 * MARGIN;
    }

    if ( !s_svgRenderer ) {
        s_svgRenderer = new QSvgRenderer( KStandardDirs::locate( "data","amarok/images/playlist_items.svg" ));
        if ( ! s_svgRenderer->isValid() )
            debug() << "svg is kaputski";
    }

    setFlag( QGraphicsItem::ItemIsSelectable );
    setFlag( QGraphicsItem::ItemIsMovable );
    setAcceptDrops( true );
   // setHandlesChildEvents( true ); // don't let drops etc hit the text items, doing stupid things
}

Playlist::GraphicsItem::~GraphicsItem()
{
    delete m_items;
}

void
Playlist::GraphicsItem::paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
// ::paint RULES:
// 1) You do not talk about ::paint method
// 2) You DO NOT talk about ::paint method
// 3) Do not show or hide item that are already shown or hidden, respectively
// 4) Do not setBrush without making sure its hasn't already been set to that brush().
// 5) If this is your first night at ::paint method, you HAVE to paint.
    Q_UNUSED( painter ); Q_UNUSED( widget );

    //debug() << "painting row: " << m_currentRow;
    const QModelIndex index = The::playlistModel()->index( m_currentRow, 0 );

    if( !m_items || ( option->rect.width() != m_items->lastWidth ) || m_groupModeChanged )
    {

        if( !m_items )
        {
            const Meta::TrackPtr track = index.data( ItemRole ).value< Playlist::Item* >()->track();
            m_items = new Playlist::GraphicsItem::ActiveItems();
            m_items->track = track;
            init( track );
        }
        m_groupModeChanged = false;
        resize( m_items->track, option->rect.width() );
    }


    if ( m_groupMode == Collapsed ) {
        // just make sure text items are hidden and then get the heck out of here...
        // whoops... this has got to be moved here below the check for (!m_items) or
        // it will evetually crasah....
        if( m_items->topRightText->isVisible() )
            m_items->topRightText->hide();
        if( m_items->topLeftText->isVisible() )
            m_items->topLeftText->hide();
        if( m_items->bottomRightText->isVisible() )
            m_items->bottomRightText->hide();
        if( m_items->bottomLeftText->isVisible() )
            m_items->bottomLeftText->hide();

        return;
    }


    // paint item background:
    QRectF trackRect;
    QRectF headRect;
    if ( ( m_groupMode == Head ) || ( m_groupMode == Head_Collapsed ) ) {

        //make the album group header stand out
        //painter->fillRect( option->rect, QBrush( Qt::darkCyan ) );
        trackRect = QRectF( option->rect.x(), ALBUM_WIDTH + 2 * MARGIN, option->rect.width(), s_fm->height() /*+ MARGIN*/ );
        if (m_groupMode == Head )
            headRect = option->rect;
        else
            headRect = QRectF( option->rect.x(), option->rect.y(), option->rect.width(), option->rect.height() - 2 );

    } else {
        trackRect = option->rect;

        if ( ( m_groupMode != Body) && !( ( m_groupMode == Head ) ) )
            trackRect.setHeight( trackRect.height() - 2 ); // add a little space between items
    }


    if ( m_groupMode == None ) {

        QString key = QString("track:%1x%2").arg(trackRect.width()).arg(trackRect.height());
        QPixmap background( (int)( trackRect.width() ), (int)( trackRect.height() ) );
        background.fill( Qt::transparent );


        if (!QPixmapCache::find(key, background)) {
            QPainter pt( &background );
            s_svgRenderer->render( &pt, "track",  trackRect );
            QPixmapCache::insert(key, background);
        }
        painter->drawPixmap( 0, 0, background );
    } else  if ( ( m_groupMode == Head ) || ( m_groupMode == Head_Collapsed ) ) {

       QString svgName;
       if ( m_groupMode == Head )
           svgName = "head";
       else 
           svgName = "collapsed_head";

        QString key = QString("%1:%2x%3").arg( svgName ).arg( headRect.width() ).arg( headRect.height() );
        QPixmap background( headRect.width(), headRect.height() );
        background.fill( Qt::transparent );

        if ( !QPixmapCache::find( key, background ) ) {
            QPainter pt( &background );
            s_svgRenderer->render( &pt, svgName,  headRect );
            QPixmapCache::insert( key, background );
        }
        painter->drawPixmap( 0, 0, background );

        //"Just for fun" stuff below this point 

        QString collapseString; 
        if ( m_groupMode == Head )
           collapseString = "collapse_button";
       else 
           collapseString = "expand_button";
          

        key = QString("%1:%2x%3").arg( collapseString ).arg( 16 ).arg( 16 );
        QPixmap collapsePixmap( 16, 16 );
        collapsePixmap.fill( Qt::transparent );

        if (!QPixmapCache::find(key, collapsePixmap)) {
            QPainter pt2( &collapsePixmap );
            s_svgRenderer->render( &pt2, collapseString,  QRectF( 0, 0, 16, 16 ) );
            QPixmapCache::insert(key, collapsePixmap);
        }

        painter->drawPixmap( option->rect.width() - ( 16 + MARGIN ) , MARGIN, collapsePixmap );
       


    } else  if ( m_groupMode == Body ) {

        QString key = QString("body:%1x%2").arg(trackRect.width()).arg(trackRect.height());
        QPixmap background( (int)( trackRect.width() ), (int)( trackRect.height() ) );
        background.fill( Qt::transparent );

        if (!QPixmapCache::find(key, background)) {
            QPainter pt( &background );
            s_svgRenderer->render( &pt, "body",  trackRect );
            QPixmapCache::insert(key, background);
        }
        painter->drawPixmap( 0, 0, background );
    } else  if ( m_groupMode == End ) {

        QString key = QString( "tail:%1x%2" ).arg( trackRect.width() ).arg(trackRect.height()) ;
        QPixmap background( (int)( trackRect.width() ), (int)( trackRect.height() ) );
        background.fill( Qt::transparent );

        if (!QPixmapCache::find(key, background)) {
            QPainter pt( &background );
            s_svgRenderer->render( &pt, "tail",  trackRect );
            QPixmapCache::insert(key, background);
        }
        painter->drawPixmap( 0, 0, background );
    }




    if ( ( m_groupMode == Head ) || ( m_groupMode == Head_Collapsed ) || ( m_groupMode == Body ) || ( m_groupMode == End ) ) {
        if ( index.data( GroupedAlternateRole ).toBool() ) {

            QString key = QString( "alternate:%1x%2" ).arg( trackRect.width() - 10 ).arg(trackRect.height() );
            QPixmap background( (int)( trackRect.width() - 10 ), (int)( trackRect.height() ) );

            QRectF tempRect = trackRect;
            tempRect.setWidth( tempRect.width() - 10 );
            if ( m_groupMode == End )
                tempRect.setHeight( tempRect.height() - 4 );

            if (!QPixmapCache::find( key, background ) ) {
                background.fill( Qt::transparent );
                QPainter pt( &background );
                s_svgRenderer->render( &pt, "body_background",  tempRect );
                QPixmapCache::insert( key, background );
            }

             if ( ( m_groupMode == Head ) || ( m_groupMode == Head_Collapsed ))
                painter->drawPixmap( 5, (int)( MARGIN + ALBUM_WIDTH + 2 ), background );
             else
                painter->drawPixmap( 5, 0, background );
        }

    }



     if ( m_groupMode < Body ) {
          //if we are not grouped, or are the head of a group, paint cover:
         QPixmap albumPixmap;
         if( m_items->track->album() )
            albumPixmap =  m_items->track->album()->image( int( ALBUM_WIDTH ) );
         painter->drawPixmap( (int)( MARGIN ), (int)( MARGIN ), albumPixmap );
         //and make sure the top text elements are shown
        if( !m_items->topRightText->isVisible() )
            m_items->topRightText->show();
        if( !m_items->topLeftText->isVisible() )
            m_items->topLeftText->show();

    } else {
        //if not, make sure that the top text items are not shown
        if( m_items->topRightText->isVisible() )
            m_items->topRightText->hide();
        if( m_items->topLeftText->isVisible() )
            m_items->topLeftText->hide();
        if( !m_items->bottomRightText->isVisible() )
            m_items->bottomRightText->show();
        if( !m_items->bottomLeftText->isVisible() )
            m_items->bottomLeftText->show();
    }

    //set selection marker if needed

    if( option->state & QStyle::State_Selected )
    {
        painter->fillRect( trackRect, QBrush( QColor( 0, 0, 255, 128 ) ) );

    }


    //set overlay if item is active:
    if( index.data( ActiveTrackRole ).toBool() )
    {
        if( !m_items->foreground )
        {
            m_items->foreground = new QGraphicsRectItem( QRectF( 0.0, 0.0, trackRect.width(), trackRect.height() ) , this );
            m_items->foreground->setPos( 0.0, trackRect.top() );
            m_items->foreground->setZValue( 10.0 );
            QRadialGradient gradient(trackRect.width() / 2.0, trackRect.height() / 2.0, trackRect.width() / 2.0, 20 + trackRect.width() / 2.0, trackRect.height() / 2.0 );
            m_items->overlayGradientStart = option->palette.highlightedText().color().light();
            m_items->overlayGradientStart.setAlpha( 80 );
            m_items->overlayGradientEnd = option->palette.highlightedText().color().dark();
            m_items->overlayGradientEnd.setAlpha( 80 );
            gradient.setColorAt( 0.0, m_items->overlayGradientStart );
            gradient.setColorAt( 1.0, m_items->overlayGradientEnd );
            QBrush brush( gradient );
            m_items->foreground->setBrush( brush );
            m_items->foreground->setPen( QPen( Qt::NoPen ) );
        }
        if( !m_items->foreground->isVisible() )
            m_items->foreground->show();
    }
    else if( m_items->foreground && m_items->foreground->isVisible() )
        m_items->foreground->hide();
}

void
Playlist::GraphicsItem::init( Meta::TrackPtr track )
{
    Q_UNUSED( track );
    QFont font;
    font.setPointSize( font.pointSize() - 1 );
    #define NewText( X ) \
        X = new Playlist::TextItem( this ); \
        X->setTextInteractionFlags( Qt::TextEditorInteraction ); \
        X->setFont( font );
    NewText( m_items->topLeftText )
    NewText( m_items->bottomLeftText )
    NewText( m_items->topRightText )
    NewText( m_items->bottomRightText )
    #undef NewText
}

void
Playlist::GraphicsItem::resize( Meta::TrackPtr track, int totalWidth )
{
    if( totalWidth == -1 /*|| totalWidth == m_items->lastWidth */) //no change needed
        return;
    if( m_items->lastWidth != -5 ) //this isn't the first "resize"
        prepareGeometryChange();
    m_items->lastWidth = totalWidth;
    QString prettyLength = Meta::secToPrettyTime( track->length() );
    QString album;
    if( track->album() )
        album = track->album()->name();

    const qreal lineTwoY = m_height / 2 + MARGIN;
    const qreal textWidth = ( ( qreal( totalWidth ) - ALBUM_WIDTH ) / 2.0 );
    const qreal leftAlignX = ALBUM_WIDTH + MARGIN;
    qreal topRightAlignX;
    qreal bottomRightAlignX;

    {
        qreal middle = textWidth + ALBUM_WIDTH + ( MARGIN * 2.0 );
        qreal rightWidth = totalWidth - qMax( s_fm->width( album )
            , s_fm->width( prettyLength ) );
        topRightAlignX = qMax( middle, rightWidth );
    }

    //lets use all the horizontal space we can get for now..
    int lengthStringWidth = (int)(s_fm->width( prettyLength ));
    bottomRightAlignX = ( totalWidth - 4 * MARGIN ) - lengthStringWidth ;




    qreal spaceForTopLeft = totalWidth - ( totalWidth - topRightAlignX ) - leftAlignX;
    qreal spaceForBottomLeft = totalWidth - ( totalWidth - bottomRightAlignX ) - leftAlignX;

    if ( m_groupMode == Head_Collapsed ) {

        m_items->bottomLeftText->setEditableText( QString("%1 tracks").arg( QString::number( m_items->groupedTracks ) ) , spaceForBottomLeft );
        QFont f = m_items->bottomLeftText->font();
        f.setItalic( true );
        m_items->bottomLeftText->setFont( f );
        int seconds = 0;
        for( int i = m_currentRow; i < m_currentRow + m_items->groupedTracks; i++ )
            seconds += The::playlistModel()->itemList()[ i ]->track()->length();
        m_items->bottomRightText->setEditableText( Meta::secToPrettyTime( seconds ), totalWidth - bottomRightAlignX );

    } else {
        m_items->bottomLeftText->setFont( m_items->bottomRightText->font() );
        m_items->bottomLeftText->setEditableText( QString("%1 - %2").arg( QString::number( track->trackNumber() ), track->name() ) , spaceForBottomLeft );
        m_items->bottomRightText->setEditableText( prettyLength, totalWidth - bottomRightAlignX );
    }
    if ( m_groupMode == None ) {

        m_items->topRightText->setPos( topRightAlignX, MARGIN );
        m_items->topRightText->setEditableText( album, totalWidth - topRightAlignX );

        {
            QString artist;
            if( track->artist() )
                artist = track->artist()->name();
            m_items->topLeftText->setEditableText( artist, spaceForTopLeft );
            m_items->topLeftText->setPos( leftAlignX, MARGIN );
        }

        m_items->bottomLeftText->setPos( leftAlignX, lineTwoY );
        m_items->bottomRightText->setPos( bottomRightAlignX, lineTwoY );
    } else if ( ( m_groupMode == Head ) || ( m_groupMode == Head_Collapsed ) ) {

        int headingCenter = (int)( MARGIN + ( ALBUM_WIDTH - s_fm->height()) / 2 );

        m_items->topRightText->setPos( topRightAlignX, headingCenter );
        m_items->topRightText->setEditableText( album, totalWidth - topRightAlignX );

        {
            QString artist;
            //various artist handling:
            //if the album has no albumartist, use Various Artists, otherwise use the albumartist's name
            if( track->album()->albumArtist() )
                artist = track->album()->albumArtist()->name();
            else
            {
                artist = findArtistForCurrentAlbum();
                if( artist.isEmpty() )
                    artist = i18n( "Various Artists" );
            }
            m_items->topLeftText->setEditableText( artist, spaceForTopLeft );
            m_items->topLeftText->setPos( leftAlignX, headingCenter );
        }

        int underImageY = (int)( MARGIN + ALBUM_WIDTH + 2 );

        m_items->bottomLeftText->setPos( MARGIN * 3, underImageY );
        m_items->bottomRightText->setPos( bottomRightAlignX, underImageY );

    } else {
        m_items->bottomLeftText->setPos( MARGIN * 3, 0 );
        m_items->bottomRightText->setPos( bottomRightAlignX, 0 );
    }

    //make sure the activ eitem overlay has the correct width



    if( m_items->foreground )
    {

        QRectF trackRect;
        if ( ( m_groupMode == Head ) || ( m_groupMode == Head_Collapsed ) ) {
            trackRect = QRectF( 0, ALBUM_WIDTH + 2 * MARGIN, totalWidth, s_fm->height() /*+ MARGIN*/ );
        } else {
            trackRect = QRectF( 0, 0, totalWidth, m_height );
            if ( ( m_groupMode != Body) && !( ( m_groupMode == Head ) ) )
                trackRect.setHeight( trackRect.height() - 2 ); // add a little space between items
        }
        
        QRadialGradient gradient(trackRect.width() / 2.0, trackRect.height() / 2.0, trackRect.width() / 2.0, 20 + trackRect.width() / 2.0, trackRect.height() / 2.0 );
        gradient.setColorAt( 0.0, m_items->overlayGradientStart );
        gradient.setColorAt( 1.0, m_items->overlayGradientEnd );
        QBrush brush( gradient );
        m_items->foreground->setRect( 0, 0, totalWidth, trackRect.height() );
        m_items->foreground->setBrush( brush );
        m_items->foreground->setPen( QPen( Qt::NoPen ) );
    }

    m_items->lastWidth = totalWidth;
}

QString
Playlist::GraphicsItem::findArtistForCurrentAlbum() const
{
    if( !( ( m_groupMode == Head ) || ( m_groupMode == Head_Collapsed ) ) )
        return QString();

    const QModelIndex index = The::playlistModel()->index( m_currentRow, 0 );
    if( ! ( ( index.data( GroupRole ).toInt() == Head ) || ( index.data( GroupRole ).toInt() == Head_Collapsed ) ) ) 
    {
        return QString();
    }
    else
    {
        QString artist;
        Meta::TrackPtr currentTrack = index.data( TrackRole ).value< Meta::TrackPtr >();
        if( currentTrack->artist() )
            artist = currentTrack->artist()->name();
        else
            return QString();
        //it's an album group, and the current row is the head, so the next row is either Body or End
        //that means we have to execute the loop at least once
        QModelIndex idx;
        int row = m_currentRow + 1;
        do
        {
            idx = The::playlistModel()->index( row++, 0 );
            Meta::TrackPtr track = idx.data( TrackRole ).value< Meta::TrackPtr >();
            if( track->artist() )
            {
                if( artist != track->artist()->name() )
                    return QString();
            }
            else
            {
                return QString();
            }
        }
        while( idx.data( GroupRole ).toInt() == Body );

        return artist;
    }
}

QRectF
Playlist::GraphicsItem::boundingRect() const
{
    // the viewport()->size() takes scrollbars into account
    return QRectF( 0.0, 0.0, The::playlistView()->viewport()->size().width(), m_height );
}

void
Playlist::GraphicsItem::play()
{
    The::playlistModel()->play( m_currentRow );
}

void
Playlist::GraphicsItem::mouseDoubleClickEvent( QGraphicsSceneMouseEvent *event )
{
    if( m_items )
    {
        event->accept();
        play();
        return;
    }
    QGraphicsItem::mouseDoubleClickEvent( event );
}

void
Playlist::GraphicsItem::mousePressEvent( QGraphicsSceneMouseEvent *event )
{
    if( event->buttons() & Qt::RightButton || !m_items )
    {
        event->ignore();
        return;
    }
    m_items->preDragLocation = mapToScene( boundingRect() ).boundingRect();

    //did we hit the collapse / expand button?

    QRectF rect( boundingRect().width() - ( 16 + MARGIN ), MARGIN, 16, 16 );
    if ( rect.contains( event->pos() ) ) {
        if ( m_groupMode == Head_Collapsed )
            The::playlistModel()->setCollapsed( m_currentRow, false );
        else if ( m_groupMode == Head )
            The::playlistModel()->setCollapsed( m_currentRow, true );
    }


    QGraphicsItem::mousePressEvent( event );
}

// With help from QGraphicsView::mouseMoveEvent()
void
Playlist::GraphicsItem::mouseMoveEvent( QGraphicsSceneMouseEvent *event )
{
    if( (event->buttons() & Qt::LeftButton) && ( flags() & QGraphicsItem::ItemIsMovable) && m_items )
    {
        QPointF scenePosition = event->scenePos();

        if( scenePosition.y() < 0 )
            return;

        bool dragOverOriginalPosition = m_items->preDragLocation.contains( scenePosition );

        //make sure item is drawn on top of other items
        setZValue( 2.0 );

        // Determine the list of selected items
        QList<QGraphicsItem *> selectedItems = scene()->selectedItems();
        if( !isSelected() )
            selectedItems << this;
        // Move all selected items
        foreach( QGraphicsItem *item, selectedItems )
        {
            if( (item->flags() & QGraphicsItem::ItemIsMovable) && (!item->parentItem() || !item->parentItem()->isSelected()) )
            {
                Playlist::GraphicsItem *above = 0;
                QPointF diff;
                if( item == this && !dragOverOriginalPosition )
                {
                    diff = event->scenePos() - event->lastScenePos();
                    QList<QGraphicsItem*> collisions = scene()->items( event->scenePos() );
                    foreach( QGraphicsItem *i, collisions )
                    {
                        Playlist::GraphicsItem *c = dynamic_cast<Playlist::GraphicsItem *>( i );
                        if( c && c != this )
                        {
                            above = c;
                            break;
                        }
                    }
                }
                else
                {
                    diff = item->mapToParent( item->mapFromScene(event->scenePos()))
                                              - item->mapToParent(item->mapFromScene(event->lastScenePos()));
                }

                item->moveBy( 0, diff.y() );
                if( item->flags() & ItemIsSelectable )
                    item->setSelected( true );

                if( dragOverOriginalPosition )
                    Playlist::DropVis::instance()->show( m_items->preDragLocation.y() );
                else
                    Playlist::DropVis::instance()->show( above );
            }
        }
    }
    else
    {
        QGraphicsItem::mouseMoveEvent( event );
    }
}

void
Playlist::GraphicsItem::dragEnterEvent( QGraphicsSceneDragDropEvent *event )
{
    foreach( QString mime, The::playlistModel()->mimeTypes() )
    {
        if( event->mimeData()->hasFormat( mime ) )
        {
            event->accept();
            Playlist::DropVis::instance()->show( this );
            break;
        }
    }
}

void
Playlist::GraphicsItem::dropEvent( QGraphicsSceneDragDropEvent * event )
{
    event->accept();
    setZValue( 1.0 );
    The::playlistModel()->dropMimeData( event->mimeData(), Qt::CopyAction, m_currentRow, 0, QModelIndex() );
    Playlist::DropVis::instance()->hide();
}

void
Playlist::GraphicsItem::refresh()
{
    QPixmap albumPixmap;
    if( !m_items || !m_items->track )
        return;

    if( m_items->track->album() )
        albumPixmap =  m_items->track->album()->image( int( ALBUM_WIDTH ) );

    //m_items->albumArt->hide();
    //delete ( m_items->albumArt );
    //m_items->albumArt = new QGraphicsPixmapItem( albumPixmap, this );
    //m_items->albumArt->setPos( 0.0, MARGIN );
}

void Playlist::GraphicsItem::mouseReleaseEvent( QGraphicsSceneMouseEvent *event )
{
    bool dragOverOriginalPosition = m_items->preDragLocation.contains( event->scenePos() );
    if( dragOverOriginalPosition )
    {
        setPos( m_items->preDragLocation.topLeft() );
        Playlist::DropVis::instance()->hide();
        return;
    }

    Playlist::GraphicsItem *above = 0;
    QList<QGraphicsItem*> collisions = scene()->items( event->scenePos() );
    foreach( QGraphicsItem *i, collisions )
    {
        Playlist::GraphicsItem *c = dynamic_cast<Playlist::GraphicsItem *>( i );
        if( c && c != this )
        {
            above = c;
            break;
        }
    }
    // if we've dropped ourself ontop of another item, then we need to shuffle the tracks below down
    if( above )
    {
        setPos( above->pos() );
        The::playlistView()->moveItem( this, above );
    } else {
        //Don't just drop item into the void, make it the last item!

        The::playlistView()->moveItem( this, 0 );
        //setPos( above->pos() );
        //The::playlistView()->moveItem( this, above );

    }

    //make sure item resets its z value
    setZValue( 1.0 );
    Playlist::DropVis::instance()->hide();
}

void Playlist::GraphicsItem::setRow(int row)
{
    //DEBUG_BLOCK
    m_currentRow = row;

    const QModelIndex index = The::playlistModel()->index( m_currentRow, 0 );

    //figure out our group state and set height accordingly
    int currentGroupState = index.data( GroupRole ).toInt();

    if ( currentGroupState != m_groupMode ) {

        debug() << "Group changed for row " << row;

        prepareGeometryChange();


        m_groupMode = currentGroupState;
        m_groupModeChanged = true;

        switch ( m_groupMode ) {

            case None:
                debug() << "None";
                m_height =  qMax( ALBUM_WIDTH, s_fm->height() * 2 ) + 2 * MARGIN + 2;
                break;
            case Head:
                debug() << "Head";
                m_height =  qMax( ALBUM_WIDTH, s_fm->height() * 2 ) + MARGIN + s_fm->height() + 4;
                break;
            case Head_Collapsed:
                debug() << "Collapsed head";
                m_height =  qMax( ALBUM_WIDTH, s_fm->height() * 2 ) + MARGIN + s_fm->height() + 10;
                if ( !m_items ) {
                    const Meta::TrackPtr track = index.data( ItemRole ).value< Playlist::Item* >()->track();
                    m_items = new Playlist::GraphicsItem::ActiveItems();
                    m_items->track = track;
                    init( track );
                }
                    m_items->groupedTracks = index.data( GroupedTracksRole ).toInt();
                break;
            case Body:
                debug() << "Body";
                m_height =  s_fm->height()/*+ 2 * MARGIN*/;
                break;
            case End:
                debug() << "End";
                m_height =  s_fm->height() + 6 /*+ 2 * MARGIN*/;
                break;
            case Collapsed:
                debug() << "Collapsed";
                m_height =  0;
                break;
            default:
                debug() << "ERROR!!??";
        }
    }
}

void Playlist::GraphicsItem::hoverEnterEvent(QGraphicsSceneHoverEvent * event)
{
    //DEBUG_BLOCK

    /*if ( m_groupMode == Head_Collapsed )
       The::playlistModel()->setCollapsed( m_currentRow, false ); */
}



