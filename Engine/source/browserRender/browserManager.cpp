//-----------------------------------------------------------------------------
// Browser Render Module for Torque3D
// MIT License - Copyright (c) 2026 Peak AI Design LLC
//-----------------------------------------------------------------------------

#include "browserRender/browserManager.h"
#include "browserRender/browserTexture.h"
#include "console/console.h"
#include "console/engineAPI.h"

#ifdef TORQUE_CEF_ENABLED
#include "include/cef_app.h"
#endif

//-----------------------------------------------------------------------------

BrowserManager *BrowserManager::smInstance = NULL;

BrowserManager::BrowserManager()
   : mInitialized(false)
{
}

BrowserManager::~BrowserManager()
{
}

BrowserManager* BrowserManager::get()
{
   if (!smInstance)
      smInstance = new BrowserManager();
   return smInstance;
}

//-----------------------------------------------------------------------------

bool BrowserManager::init()
{
   if (mInitialized)
      return true;

#ifdef TORQUE_CEF_ENABLED
   Con::printf("BrowserManager: Initializing CEF...");

   CefMainArgs args;

   CefSettings settings;
   settings.windowless_rendering_enabled = true;
   settings.no_sandbox = true;
   // Use a subdirectory for CEF cache.
   CefString(&settings.cache_path).FromASCII("cache/cef");
   // Log to console.
   CefString(&settings.log_file).FromASCII("cef_debug.log");
   settings.log_severity = LOGSEVERITY_WARNING;

   // We run CEF in single-process mode for simplicity.
   // For production, you'd want multi-process with a separate
   // subprocess executable.
   settings.multi_threaded_message_loop = false;
   settings.external_message_pump = true;

   if (!CefInitialize(args, settings, nullptr, nullptr))
   {
      Con::errorf("BrowserManager: CefInitialize failed!");
      return false;
   }

   mInitialized = true;
   Con::printf("BrowserManager: CEF initialized successfully.");
   return true;

#else
   Con::warnf("BrowserManager: CEF support not compiled in (TORQUE_CEF_ENABLED not defined).");
   mInitialized = false;
   return false;
#endif
}

//-----------------------------------------------------------------------------

void BrowserManager::shutdown()
{
   if (!mInitialized)
      return;

   Con::printf("BrowserManager: Shutting down CEF...");

   // Clear all browser references.
   mBrowsers.clear();

#ifdef TORQUE_CEF_ENABLED
   CefShutdown();
#endif

   mInitialized = false;

   if (smInstance)
   {
      delete smInstance;
      smInstance = NULL;
   }
}

//-----------------------------------------------------------------------------

void BrowserManager::process()
{
   if (!mInitialized)
      return;

#ifdef TORQUE_CEF_ENABLED
   // Pump the CEF message loop (external message pump mode).
   CefDoMessageLoopWork();
#endif

   // Tick all active browser textures.
   for (U32 i = 0; i < mBrowsers.size(); i++)
   {
      if (mBrowsers[i])
         mBrowsers[i]->processTick();
   }
}

//-----------------------------------------------------------------------------

void BrowserManager::addBrowser(BrowserTexture *browser)
{
   // Auto-init CEF on first browser creation.
   if (!mInitialized)
      init();

   mBrowsers.push_back(browser);
}

void BrowserManager::removeBrowser(BrowserTexture *browser)
{
   for (U32 i = 0; i < mBrowsers.size(); i++)
   {
      if (mBrowsers[i] == browser)
      {
         mBrowsers.erase(i);
         return;
      }
   }
}

//-----------------------------------------------------------------------------
// Console functions
//-----------------------------------------------------------------------------

DefineEngineFunction(browserInit, bool, (), ,
   "Initialize the browser rendering subsystem (CEF).\n"
   "@return True if initialization succeeded.\n")
{
   return BrowserManager::get()->init();
}

DefineEngineFunction(browserShutdown, void, (), ,
   "Shut down the browser rendering subsystem.\n")
{
   BrowserManager::get()->shutdown();
}

DefineEngineFunction(browserGetCount, S32, (), ,
   "Get the number of active browser instances.\n"
   "@return The browser count.\n")
{
   return BrowserManager::get()->getBrowserCount();
}

DefineEngineFunction(isBrowserAvailable, bool, (), ,
   "Check if the browser subsystem is available and initialized.\n"
   "@return True if CEF is running.\n")
{
   return BrowserManager::get()->isInitialized();
}
