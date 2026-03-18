//-----------------------------------------------------------------------------
// Browser Render Module for Torque3D
// MIT License - Copyright (c) 2026 Peak AI Design LLC
//
// BrowserManager - Singleton that owns the CEF lifecycle and pumps
// the message loop. All BrowserTexture instances register here for
// per-frame updates.
//-----------------------------------------------------------------------------

#ifndef _BROWSER_MANAGER_H_
#define _BROWSER_MANAGER_H_

#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif
#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif

class BrowserTexture;

/// Singleton manager for the browser rendering subsystem.
/// Initialises/shuts down CEF and ticks all active BrowserTexture instances.
class BrowserManager
{
public:
   /// Access the singleton.
   static BrowserManager* get();

   /// Initialise CEF. Call once during engine startup.
   bool init();

   /// Shut down CEF. Call during engine shutdown.
   void shutdown();

   /// Pump the CEF message loop and update all browser textures.
   /// Called once per frame from the main game loop.
   void process();

   /// Register/unregister a BrowserTexture for per-frame updates.
   void addBrowser(BrowserTexture *browser);
   void removeBrowser(BrowserTexture *browser);

   /// Has CEF been initialised?
   bool isInitialized() const { return mInitialized; }

   /// Get the number of active browser instances.
   U32 getBrowserCount() const { return mBrowsers.size(); }

private:
   BrowserManager();
   ~BrowserManager();

   static BrowserManager *smInstance;

   bool mInitialized;

   /// All active browser textures.
   Vector<BrowserTexture*> mBrowsers;
};

#endif // _BROWSER_MANAGER_H_
