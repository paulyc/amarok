/*
 *
 * This software is released under the provisions of the GPL version 2.
 * see file "COPYING".  If that file is not available, the full statement 
 * of the license can be found at
 *
 * http://www.fsf.org/licensing/licenses/gpl.txt
 *
 * Portions Copyright (c) 1995-2002 RealNetworks, Inc. All Rights Reserved.
 * Portions (c) Paul Cifarelli 2005
 *
 */

#ifndef _HELIX_SIMPLEPLAYER_LIB_H_INCLUDED_
#define _HELIX_SIMPLEPLAYER_LIB_H_INCLUDED_

class HSPClientAdviceSink;
class HSPClientContext;
class IHXErrorSinkControl;
class HelixSimplePlayerAudioStreamInfoResponse;

#include <limits.h>
#include <sys/param.h>
#include <pthread.h>
#define MAX_PATH PATH_MAX

#define MAX_PLAYERS 100 // that should do it...
#define MAX_SCOPE_SAMPLES 5120

class HelixSimplePlayer;
class CHXURL;
class IHXVolumeAdviseSink;
class IHXAudioPlayer;
class IHXAudioStream;
class IHXAudioCrossFade;
class IHXPlayer;
class IHXErrorSink;
class IHXErrorSinkControl;
class IHXAudioPlayer;
class IHXVolume;
class IHXPlayerNavigator;
class IHXClientEngineSelector;
class IHXClientEngine;

struct DelayQueue
{
   struct DelayQueue *fwd;
   unsigned long time;
   int buf[512];
};

class HelixSimplePlayer
{
public:
   enum { ALL_PLAYERS = -1 };

   HelixSimplePlayer();
   virtual ~HelixSimplePlayer();

   void init(const char *corelibpath, const char *pluginslibpath, const char *codecspath, int numPlayers = 1);
   int  addPlayer();                                                 // add another player
   void play(int playerIndex = ALL_PLAYERS);                         // play the current url, waiting for it to finish
   void play(const char *url, int playerIndex = ALL_PLAYERS);        // play the file, setting it as the current url; wait for it to finish
   int  setURL(const char *url, int playerIndex = ALL_PLAYERS);      // set the current url
   bool done(int playerIndex = ALL_PLAYERS);                         // test to see if the player(s) is(are) done
   void start(int playerIndex = ALL_PLAYERS);                        // start the player
   void start(const char *file, int playerIndex = ALL_PLAYERS);      // start the player, setting the current url first
   void stop(int playerIndex = ALL_PLAYERS);                         // stop the player(s)
   void pause(int playerIndex = ALL_PLAYERS);                        // pause the player(s)
   void resume(int playerIndex = ALL_PLAYERS);                       // pause the player(s)
   void seek(unsigned long pos, int playerIndex = ALL_PLAYERS);      // seek to the pos
   unsigned long where(int playerIndex) const;                       // where is the player in the playback
   unsigned long duration(int playerIndex) const;                    // how long (ms) is this clip?
   unsigned long getVolume(int playerIndex);                         // get the current volume
   //void initVolume(unsigned short min, unsigned short max, int playerIndex); // called to set the range of the clients volume - mapped onto the device
   void setVolume(unsigned long vol, int playerIndex = ALL_PLAYERS); // set the volume
   void setMute(bool mute, int playerIndex = ALL_PLAYERS);           // set mute: mute = true to mute the volume, false to unmute
   bool getMute(int playerIndex);                                    // get the mute state of the player
   void dispatch();                                                  // dispatch the player(s)

   virtual void onVolumeChange(int) {}                   // called when the volume is changed
   virtual void onMuteChange(int) {}                     // called when mute is changed

   int getError() const { return theErr; }

   static char* RemoveWrappingQuotes(char* str);
   //void  setUsername(const char *username) { m_pszUsername = username; }
   //void  setPassword(const char *password) { m_pszPassword = password; }
   //void  setGUIDFile(const char *file) { m_pszGUIDFile = file; }
   bool  ReadGUIDFile();
   //static struct _stGlobals*& GetGlobal();

private:
   void  DoEvent();
   void  DoEvents(int nTimeDelta);
   unsigned long GetTime();

   char                    mCoreLibPath[MAXPATHLEN];
   char                    mPluginLibPath[MAXPATHLEN];
   int                     theErr;
   HSPClientContext**      ppHSPContexts;
   IHXPlayer**             ppPlayers;
   IHXErrorSink*           pErrorSink;
   IHXErrorSinkControl*    pErrorSinkControl;
   IHXAudioPlayer**        ppAudioPlayer;
   IHXAudioCrossFade**     ppCrossFader;
   IHXVolume**             ppVolume;
   IHXPlayerNavigator*     pPlayerNavigator;
   IHXClientEngineSelector *pCEselect;
   char**                  ppszURL;
   bool                    bURLFound;
   int                     nNumPlayers;
   int                     nNumPlayRepeats;
   int                     nTimeDelta;
   int                     nStopTime;
   bool		           bStopTime;
   void*                   core_handle;
   bool 		   bStopping;
   int 		           nPlay;

   // crossfading
   struct xfade
   {
      bool                 crossfading;
      unsigned long        duration;
      int                  fromIndex;
      int                  toIndex;
      IHXAudioStream       *fromStream;
      IHXAudioStream       *toStream;
   } m_xf;
   
public:
   // cross fade the next track (given by url). startPos is in the old stream
   void enableCrossFader(int playerFrom, int playerTo);
   void disableCrossFader();
   void crossFade(const char *url, unsigned long startPos, unsigned long xfduration);
   inline struct xfade &xf() { return m_xf; }
   const IHXAudioPlayer *getAudioPlayer(int playerIndex) const { return ppAudioPlayer[playerIndex]; }
   const IHXAudioCrossFade *getCrossFader(int playerIndex) const { return ppCrossFader[playerIndex]; }
   void startCrossFade();

   // scope
   void addScopeBuf(struct DelayQueue *item);
   struct DelayQueue *getScopeBuf();
   int getScopeCount() { return scopecount; }
   int peekScopeTime(unsigned long &t);
   void clearScopeQ();

private:

   bool                 bEnableAdviceSink;
   bool                 bEnableVerboseMode;
   IHXClientEngine*     pEngine;   
   char*                m_pszUsername;
   char*                m_pszPassword;
   char*                m_pszGUIDFile;
   char*                m_pszGUIDList;
   int                  m_Error;
   unsigned long        m_ulNumSecondsPlayed;

   int                  scopecount;
   struct DelayQueue   *scopebufhead;
   struct DelayQueue   *scopebuftail;
   pthread_mutex_t      m_scope_m;
   friend class HSPClientAdviceSink;
   friend class HSPErrorSink;
   friend class HSPAuthenticationManager;
   friend class HelixSimplePlayerAudioStreamInfoResponse;
};

#endif
