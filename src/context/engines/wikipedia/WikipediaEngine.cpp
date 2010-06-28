/****************************************************************************************
 * Copyright (c) 2007 Leo Franchi <lfranchi@gmail.com>                                  *
 * Copyright (c) 2008 Mark Kretschmann <kretschmann@kde.org>                            *
 * Copyright (c) 2009 Simon Esneault <simon.esneault@gmail.com>                         *
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

#define DEBUG_PREFIX "WikipediaEngine"

#include "WikipediaEngine.h"

#include "core/support/Amarok.h"
#include "core/support/Debug.h"
#include "ContextObserver.h"
#include "ContextView.h"
#include "EngineController.h"

#include <Plasma/DataContainer>

using namespace Context;

class WikipediaEnginePrivate
{
private:
    WikipediaEngine *const q_ptr;
    Q_DECLARE_PUBLIC( WikipediaEngine )

public:
    WikipediaEnginePrivate( WikipediaEngine *parent )
        : q_ptr( parent )
        , currentSelection( "artist" )
        , requested( true )
        , sources( "current" )
        , wikiWideLang( "aut" )
        , triedRefinedSearch( 0 )
        , dataContainer( 0 )
    {}
    ~WikipediaEnginePrivate() {}

    // functions
    void updateEngine();

    QString wikiArtistPostfix();
    QString wikiAlbumPostfix();
    QString wikiTrackPostfix();
    QUrl wikiUrl( const QString& item ) const;
    QString wikiLocale() const;

    QString wikiParse();

    void reloadWikipedia();

    // data members
    QString currentSelection;
    bool requested;
    QStringList sources;
    QString wiki;
    QString wikiCurrentEntry;
    QString wikiCurrentLastEntry;
    QUrl wikiCurrentUrl;
    QString wikiLanguages;
    QLocale wikiLang;
    QString wikiWideLang;
    short triedRefinedSearch;

    Plasma::DataContainer *dataContainer;

    QSet< QUrl > urls;

    // private slots
    void _dataContainerUpdated( const QString &source, const Plasma::DataEngine::Data &data );
    void _wikiResult( const KUrl &url, QByteArray result, NetworkAccessManagerProxy::Error e );
};

void
WikipediaEnginePrivate::_dataContainerUpdated( const QString &source, const Plasma::DataEngine::Data &data )
{
    Q_Q( WikipediaEngine );

    if( source != "wikipedia" )
        return;

    if( data.value( "reload" ).toBool() )
    {
        debug() << "reloading";
        dataContainer->setData( "reload", false );
        reloadWikipedia();
        return;
    }

    QString gotoType = data.value( "goto" ).toString();
    if( !gotoType.isEmpty() )
    {
        debug() << "goto:" << gotoType;
        q->setSelection( gotoType );
        updateEngine();
        return;
    }

    QUrl clickUrl = data.value( "clickUrl" ).toUrl();
    if( clickUrl.isValid() )
    {
        debug() << "clickUrl:" << clickUrl;
        wikiCurrentUrl = clickUrl;
        if( !wikiCurrentUrl.hasQueryItem( "useskin" ) )
            wikiCurrentUrl.addQueryItem( "useskin", "monobook" );
        urls << wikiCurrentUrl;
        q->setData( source, "busy", "busy" );
        The::networkAccessManager()->getData( wikiCurrentUrl, q,
             SLOT(_wikiResult(KUrl,QByteArray,NetworkAccessManagerProxy::Error)) );
        return;
    }

    // TODO: QString lang = data.value( "lang" ).toString();
}

void
WikipediaEnginePrivate::_wikiResult( const KUrl &url, QByteArray result, NetworkAccessManagerProxy::Error e )
{
    Q_Q( WikipediaEngine );
    if( !urls.contains( url ) )
        return;

    urls.remove( url );
    if( e.code != QNetworkReply::NoError )
    {
        q->setData( "wikipedia", "message", i18n("Unable to retrieve Wikipedia information: %1", e.description) );
        return;
    }

    debug() << "Received page from wikipedia:" << url;
    wiki = result;

    // FIXME: For now we test if we got an article or not with a test on this string "wgArticleId=0"
    // This is bad
    if( wiki.contains("wgArticleId=0") &&
        (wiki.contains("wgNamespaceNumber=0") ||
         wiki.contains("wgPageName=\"Special:Badtitle\"") ) ) // The article does not exist
    {
        // Refined search is done here
        if( triedRefinedSearch == -1 )
        {
            debug() << "We already tried some refined search. Lets end this madness...";
            q->removeAllData( "wikipedia" );
            q->scheduleSourcesUpdated();
            q->setData( "wikipedia", "message", i18n( "No information found..." ) );
            return;
        }
        triedRefinedSearch++;
        QString entry;
        debug() << "Article not found. Retrying with refinements.";


        if( wikiCurrentEntry.lastIndexOf( "(" ) != -1)
            wikiCurrentEntry.remove( wikiCurrentEntry.lastIndexOf( "(" )-1, wikiCurrentEntry.size() );

        if( q->selection() == "artist" )
            entry = wikiCurrentEntry+= wikiArtistPostfix() ;
        else if ( q->selection() == "album" )
            entry = wikiCurrentEntry+= wikiAlbumPostfix() ;
        else if ( q->selection() == "track" )
            entry = wikiCurrentEntry+= wikiTrackPostfix() ;

        wikiCurrentUrl = wikiUrl( entry );
        reloadWikipedia();
        return;
    }

    // We've find a page
    q->removeAllData( "wikipedia" );
    q->scheduleSourcesUpdated();

    DataEngine::Data data;
    data["page"] = wikiParse();
    data["url"] = wikiCurrentUrl;

    Meta::TrackPtr currentTrack = The::engineController()->currentTrack();
    if( currentTrack )
    {
        if( q->selection() == "artist" ) // default, or applet told us to fetch artist
        {
            if( currentTrack->artist() )
            {
                data["label"] =  "Artist";
                data["title"] = currentTrack->artist()->prettyName();
            }
        }
        else if( q->selection() == "track" )
        {
            data["label"] = "Title";
            data["title"] = currentTrack->prettyName();
        }
        else if( q->selection() == "album" )
        {
            if( currentTrack->album() )
            {
                data["label"] = "Album";
                data["title"] = currentTrack->album()->prettyName();
            }
        }
    }
    q->setData( "wikipedia", data );
}

void
WikipediaEnginePrivate::updateEngine()
{
    DEBUG_BLOCK
    Q_Q( WikipediaEngine );

    // We've got a new track, great, let's fetch some info from Wikipedia !
    triedRefinedSearch = 0;
    QString tmpWikiStr;


    Meta::TrackPtr currentTrack = The::engineController()->currentTrack();
    if( !currentTrack )
        return;

    // default, or applet told us to fetch artist
    if( q->selection() == "artist" )
    {
        if( currentTrack->artist() )
        {
            if ( currentTrack->artist()->prettyName().isEmpty() )
            {
                debug() << "Requesting an empty string, skipping !";
                q->removeAllData( "wikipedia" );
                q->scheduleSourcesUpdated();
                q->setData( "wikipedia", "message", i18n( "No information found..." ) );
                return;
            }
            if ( ( currentTrack->playableUrl().protocol() == "lastfm" ) ||
                ( currentTrack->playableUrl().protocol() == "daap" ) ||
                !The::engineController()->isStream() )
                tmpWikiStr = currentTrack->artist()->name() + wikiArtistPostfix();
            else
                tmpWikiStr = currentTrack->artist()->prettyName() + wikiArtistPostfix();
        }
    }
    else if( q->selection() == "album" )
    {
        if( currentTrack->album() )
        {
            if( currentTrack->album()->prettyName().isEmpty() )
            {
                debug() << "Requesting an empty string, skipping !";
                q->removeAllData( "wikipedia" );
                q->scheduleSourcesUpdated();
                q->setData( "wikipedia", "message", i18n( "No information found..." ) );
                return;
            }
            if( ( currentTrack->playableUrl().protocol() == "lastfm" ) ||
                ( currentTrack->playableUrl().protocol() == "daap" ) ||
                !The::engineController()->isStream() )
                tmpWikiStr = currentTrack->album()->name() + wikiAlbumPostfix();

        }
    }
    else if( q->selection() == "track" )
    {
        if( currentTrack->prettyName().isEmpty() )
        {
            debug() << "Requesting an empty string, skipping !";
            q->removeAllData( "wikipedia" );
            q->scheduleSourcesUpdated();
            q->setData( "wikipedia", "message", i18n( "No information found..." ) );
            return;
        }
        tmpWikiStr = currentTrack->prettyName() + wikiTrackPostfix();
    }
    //Hack to make wiki searches work with magnatune preview tracks
    if( tmpWikiStr.contains( "PREVIEW: buy it at www.magnatune.com" ) )
    {
        tmpWikiStr = tmpWikiStr.remove(" (PREVIEW: buy it at www.magnatune.com)" );

        int index = tmpWikiStr.indexOf( '-' );
        if( index != -1 )
            tmpWikiStr = tmpWikiStr.left (index - 1);
    }

    if( wikiCurrentLastEntry == tmpWikiStr )
    {
        debug() << "Same entry requested again. Ignoring.";
        return;
    }

    q->removeAllData( "wikipedia" );
    q->scheduleSourcesUpdated();

    wikiCurrentLastEntry = tmpWikiStr;
    wikiCurrentEntry = tmpWikiStr;
    wikiCurrentUrl = wikiUrl( tmpWikiStr );

    debug() << "wiki url: " << wikiCurrentUrl;

    if( wikiCurrentUrl.isValid() )
    {
        urls << wikiCurrentUrl;
        The::networkAccessManager()->getData( wikiCurrentUrl, q,
             SLOT(_wikiResult(KUrl,QByteArray,NetworkAccessManagerProxy::Error)) );
    }
}

QString
WikipediaEnginePrivate::wikiParse()
{
    //remove the new-lines and tabs(replace with spaces IS needed).
    wiki.replace( '\n', ' ' );
    wiki.replace( '\t', ' ' );

    wikiLanguages.clear();
    // Get the available language list
    if ( wiki.indexOf("<div id=\"p-lang\" class=\"portlet\">") != -1 )
    {
        wikiLanguages = wiki.mid( wiki.indexOf("<div id=\"p-lang\" class=\"portlet\">") );
        wikiLanguages = wikiLanguages.mid( wikiLanguages.indexOf("<ul>") );
        wikiLanguages = wikiLanguages.mid( 0, wikiLanguages.indexOf( "</div>" ) );
    }

    QString copyright;
    QString copyrightMark = "<li id=\"f-copyright\">";
    if ( wiki.indexOf( copyrightMark ) != -1 )
    {
        copyright = wiki.mid( wiki.indexOf(copyrightMark) + copyrightMark.length() );
        copyright = copyright.mid( 0, copyright.indexOf( "</li>" ) );
        copyright.remove( "<br />" );
        //only one br at the beginning
        copyright.prepend( "<br />" );
    }

    // Ok lets remove the top and bottom parts of the page
    wiki = wiki.mid( wiki.indexOf( "<!-- start content -->" ) );
    wiki = wiki.mid( 0, wiki.indexOf( "<div class=\"printfooter\">" ) );

    // lets remove the warning box
    QString mbox = "<table class=\"metadata plainlinks ambox";
    QString mboxend = "</table>";
    while ( wiki.indexOf( mbox ) != -1 )
        wiki.remove( wiki.indexOf( mbox ), wiki.mid( wiki.indexOf( mbox ) ).indexOf( mboxend ) + mboxend.size() );

    QString protec = "<div><a href=\"/wiki/Wikipedia:Protection_policy" ;
    QString protecend = "</a></div>" ;
    while ( wiki.indexOf( protec ) != -1 )
        wiki.remove( wiki.indexOf( protec ), wiki.mid( wiki.indexOf( protec ) ).indexOf( protecend ) + protecend.size() );

    // lets also remove the "lock" image
    QString topicon = "<div class=\"metadata topicon\" " ;
    QString topiconend = "</a></div>";
     while ( wiki.indexOf( topicon ) != -1 )
        wiki.remove( wiki.indexOf( topicon ), wiki.mid( wiki.indexOf( topicon ) ).indexOf( topiconend ) + topiconend.size() );


    // Adding back style and license information
    wiki = "<div id=\"bodyContent\"" + wiki;
    wiki += copyright;
    wiki.append( "</div>" );
    wiki.remove( QRegExp("<h3 id=\"siteSub\">[^<]*</h3>") );

    wiki.remove( QRegExp( "<span class=\"editsection\"[^>]*>[^<]*<[^>]*>[^<]*<[^>]*>[^<]*</span>" ) );

    wiki.replace( QRegExp( "<a href=\"[^\"]*\" class=\"new\"[^>]*>([^<]*)</a>" ), "\\1" );

    // Remove anything inside of a class called urlexpansion, as it's pointless for us
    wiki.remove( QRegExp( "<span class= *'urlexpansion'>[^(]*[(][^)]*[)]</span>" ) );

    // Remove hidden table rows as well
    QRegExp hidden( "<tr *class= *[\"\']hiddenStructure[\"\']>.*</tr>", Qt::CaseInsensitive );
    hidden.setMinimal( true ); //greedy behaviour wouldn't be any good!
    wiki.remove( hidden );

    // we want to keep our own style (we need to modify the stylesheet a bit to handle things nicely)
    wiki.remove( QRegExp( "style= *\"[^\"]*\"" ) );
    // We need to leave the classes behind, otherwise styling it ourselves gets really nasty and tedious and roughly impossible to do in a sane maner
    //wiki.replace( QRegExp( "class= *\"[^\"]*\"" ), QString() );
    // let's remove the form elements, we don't want them.
    wiki.remove( QRegExp( "<input[^>]*>" ) );
    wiki.remove( QRegExp( "<select[^>]*>" ) );
    wiki.remove( "</select>\n"  );
    wiki.remove( QRegExp( "<option[^>]*>" ) );
    wiki.remove( "</option>\n"  );
    wiki.remove( QRegExp( "<textarea[^>]*>" ) );
    wiki.remove( "</textarea>" );

    QString m_wikiHTMLSource = "<html><body>\n";
    m_wikiHTMLSource.append( wiki );
    if ( !wikiLanguages.isEmpty() )
    {
        m_wikiHTMLSource.append( "<br/><div id=\"wiki_otherlangs\" >" + i18n( "Wikipedia Other Languages: <br/>" )+ wikiLanguages + " </div>" );
    }
    m_wikiHTMLSource.append( "</body></html>\n" );

    return m_wikiHTMLSource;
}

inline QString
WikipediaEnginePrivate::wikiLocale() const
{
    // if there is no language set (QLocale::C) then return english as default
    if( wikiWideLang == "aut" )
    {
        if( wikiLang.language() == QLocale::C )
            return "en";
        else
            return wikiLang.name().split( '_' )[0];
    }
    else
        return wikiWideLang;
}

inline QString
WikipediaEnginePrivate::wikiArtistPostfix()
{
    // prepare every case in each langage, if it's the last option, set triedRefinedSearch to -1

    if( wikiLocale() == "en" )
    {
        switch ( triedRefinedSearch )
        {
            case 0:
                return "_(band)";
            case 1:
                return "_(musician)";
            case 2:
                return "_(singer)";
            default:
                triedRefinedSearch = -1;
                return "";
        }
    }
    else if( wikiLocale() == "de" )
    {
        switch ( triedRefinedSearch )
        {
            case 0:
                return "_(Band)";
            case 1:
                return "_(Musiker)";
            default:
                triedRefinedSearch = -1;
                return "";
        }
    }
    else if( wikiLocale() == "pl" )
    {
        switch ( triedRefinedSearch )
        {
            case 0:
                return "_(grupa muzyczna)";
            default:
                triedRefinedSearch = -1;
                return "";
        }
    }
    else if( wikiLocale() == "fr" )
    {
        switch ( triedRefinedSearch )
        {
            case 0:
                return "_(groupe)";
            case 1:
                return "_(musicien)";
            case 2:
                return "_(chanteur)";
            case 3:
                return "_(chanteuse)";
            default:
                triedRefinedSearch = -1;
                return "";
        }
    }
    else // for every other country
    {
        triedRefinedSearch = -1;
        return "";
    }
}

inline QString
WikipediaEnginePrivate::wikiAlbumPostfix()
{
    Meta::TrackPtr currentTrack = The::engineController()->currentTrack();
    QString artist = currentTrack ? currentTrack->artist()->prettyName() : QString();

    if( wikiLocale() == "en" )
    {
        switch ( triedRefinedSearch )
        {
            case 0:
                return QString(" (") + artist + QString(" album)");
            case 1:
                return "_(album)";
            case 2:
                return "_(score)";
            default:
                triedRefinedSearch = -1;
                return "";
        }
    }
    else if( wikiLocale() == "fr" )
    {
        switch ( triedRefinedSearch )
        {
            case 0:
                return QString(" (") + artist + QString(" album)");
            case 1:
                return "_(album)";
            case 2:
                return "_(BO)";
            default:
                triedRefinedSearch = -1;
                return "";
        }
    }
    else
    {
        switch ( triedRefinedSearch )
        {
            case 0:
                return QString(" (") + artist + QString(" album)");
            case 1:
                return "_(album)";
            default:
                triedRefinedSearch = -1;
                return "";
        }
    }
}

inline QString
WikipediaEnginePrivate::wikiTrackPostfix()
{
    if( wikiLocale() == "en" )
    {
        Meta::TrackPtr currentTrack = The::engineController()->currentTrack();
        QString artist = currentTrack ? currentTrack->artist()->prettyName() : QString();
        switch ( triedRefinedSearch )
        {
            case 0:
                return QString(" (") + artist + QString(" song)");
            case 1:
                return "_(song)";
            default:
                triedRefinedSearch = -1;
                return "";
        }
    }
    else
    {
        triedRefinedSearch = -1;
        return "";
    }
}

inline QUrl
WikipediaEnginePrivate::wikiUrl( const QString &item ) const
{
    // We now use:  http://en.wikipedia.org/w/index.php?title=The_Beatles&useskin=monobook
    // instead of:  http://en.wikipedia.org/wiki/The_Beatles
    // So that wikipedia skin is forced to default "monoskin", and the page can be parsed correctly (see BUG 205901 )
    QUrl url;
    url.setScheme( "http" );
    url.setHost( wikiLocale() + ".wikipedia.org" );
    url.setPath( "/w/index.php" );
    url.addQueryItem( "useskin", "monobook" );

    QString text = item;
    text.replace( QChar(' '), QChar('_') );
    url.addQueryItem( "title", text  );
    return url;
}

void
WikipediaEnginePrivate::reloadWikipedia()
{
    DEBUG_BLOCK
    Q_Q( WikipediaEngine );

    debug() << "wiki url: " << wikiCurrentUrl;

    urls << wikiCurrentUrl;
    The::networkAccessManager()->getData( wikiCurrentUrl, q,
         SLOT(_wikiResult(KUrl,QByteArray,NetworkAccessManagerProxy::Error)) );
}

WikipediaEngine::WikipediaEngine( QObject* parent, const QList<QVariant>& /*args*/ )
    : DataEngine( parent )
    , ContextObserver( ContextView::self() )
    , d_ptr( new WikipediaEnginePrivate( this ) )
{
}

WikipediaEngine::~WikipediaEngine()
{
    delete d_ptr;
}

void
WikipediaEngine::init()
{
    Q_D( WikipediaEngine );
    d->dataContainer = new Plasma::DataContainer( this );
    d->dataContainer->setObjectName( "wikipedia" );
    addSource( d->dataContainer );
    connect( d->dataContainer, SIGNAL(dataUpdated(QString,Plasma::DataEngine::Data)),
             this, SLOT(_dataContainerUpdated(QString,Plasma::DataEngine::Data)) );
    d->updateEngine();
}

QStringList
WikipediaEngine::sources() const
{
    Q_D( const WikipediaEngine );
    return d->sources;
}

bool
WikipediaEngine::sourceRequestEvent( const QString &source )
{
    DEBUG_BLOCK
    Q_D( WikipediaEngine );

    d->requested = true; // someone is asking for data, so we turn ourselves on :)
    QUrl url;

    if( source == "update" )
    {
        debug() << "scheduleSourcesUpdated";
        scheduleSourcesUpdated();
    }
    else if( source.startsWith( "lang:" ) )
    {
        // user has selected is favorite language.
        d->wikiWideLang = source.mid( 5 );
    }
    else
    {
        // otherwise, it comes from the engine, a new track is playing.
        removeAllData( source );
        scheduleSourcesUpdated();
        setData( source, Plasma::DataEngine::Data() );
        d->updateEngine();
    }
    return false;
}

void
WikipediaEngine::message( const ContextState& state )
{
    Q_D( WikipediaEngine );
    if( state == Current && d->requested )
        d->updateEngine();
}

void
WikipediaEngine::metadataChanged( Meta::TrackPtr track )
{
    DEBUG_BLOCK
    Q_UNUSED( track )
    Q_D( WikipediaEngine );

    d->updateEngine();
}

QString
WikipediaEngine::selection() const
{
    Q_D( const WikipediaEngine );
    return d->currentSelection;
}

void
WikipediaEngine::setSelection( const QString &selection )
{
    Q_D( WikipediaEngine );
    d->currentSelection = selection;
}

#include "WikipediaEngine.moc"

