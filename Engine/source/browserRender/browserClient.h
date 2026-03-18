//-----------------------------------------------------------------------------
// Browser Render Module for Torque3D
// MIT License - Copyright (c) 2026 Peak AI Design LLC
//
// CefClient implementation. Wires up CEF's browser callbacks including
// the off-screen render handler, life span handler, and load handler.
//-----------------------------------------------------------------------------

#ifndef _BROWSER_CLIENT_H_
#define _BROWSER_CLIENT_H_

#ifdef TORQUE_CEF_ENABLED

#include "include/cef_client.h"
#include "include/cef_life_span_handler.h"
#include "include/cef_load_handler.h"

#include "browserRender/browserRenderHandler.h"

/// CefClient that ties a single CEF browser instance to our
/// off-screen render handler.
class BrowserCefClient : public CefClient,
                         public CefLifeSpanHandler,
                         public CefLoadHandler
{
public:
   BrowserCefClient(BrowserRenderHandler *renderHandler);
   virtual ~BrowserCefClient();

   // CefClient interface
   CefRefPtr<CefRenderHandler> GetRenderHandler() override
   {
      return mRenderHandler;
   }

   CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override
   {
      return this;
   }

   CefRefPtr<CefLoadHandler> GetLoadHandler() override
   {
      return this;
   }

   // CefLifeSpanHandler interface
   void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
   void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

   // CefLoadHandler interface
   void OnLoadEnd(CefRefPtr<CefBrowser> browser,
                  CefRefPtr<CefFrame> frame,
                  int httpStatusCode) override;

   void OnLoadError(CefRefPtr<CefBrowser> browser,
                    CefRefPtr<CefFrame> frame,
                    ErrorCode errorCode,
                    const CefString &errorText,
                    const CefString &failedUrl) override;

   /// Get the managed browser instance.
   CefRefPtr<CefBrowser> getBrowser() const { return mBrowser; }

   /// Is the browser created and alive?
   bool isValid() const { return mBrowser.get() != nullptr; }

private:
   CefRefPtr<BrowserRenderHandler> mRenderHandler;
   CefRefPtr<CefBrowser> mBrowser;

   IMPLEMENT_REFCOUNTING(BrowserCefClient);
};

#endif // TORQUE_CEF_ENABLED
#endif // _BROWSER_CLIENT_H_
