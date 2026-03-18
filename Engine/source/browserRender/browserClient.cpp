//-----------------------------------------------------------------------------
// Browser Render Module for Torque3D
// MIT License - Copyright (c) 2026 Peak AI Design LLC
//-----------------------------------------------------------------------------

#include "browserRender/browserClient.h"

#ifdef TORQUE_CEF_ENABLED

#include "console/console.h"

//-----------------------------------------------------------------------------

BrowserCefClient::BrowserCefClient(BrowserRenderHandler *renderHandler)
   : mRenderHandler(renderHandler)
{
}

BrowserCefClient::~BrowserCefClient()
{
}

//-----------------------------------------------------------------------------
// CefLifeSpanHandler
//-----------------------------------------------------------------------------

void BrowserCefClient::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
   mBrowser = browser;
   Con::printf("BrowserCefClient: Browser created (id=%d)", browser->GetIdentifier());
}

void BrowserCefClient::OnBeforeClose(CefRefPtr<CefBrowser> browser)
{
   if (mBrowser && mBrowser->GetIdentifier() == browser->GetIdentifier())
   {
      Con::printf("BrowserCefClient: Browser closed (id=%d)", browser->GetIdentifier());
      mBrowser = nullptr;
   }
}

//-----------------------------------------------------------------------------
// CefLoadHandler
//-----------------------------------------------------------------------------

void BrowserCefClient::OnLoadEnd(CefRefPtr<CefBrowser> browser,
                                  CefRefPtr<CefFrame> frame,
                                  int httpStatusCode)
{
   if (frame->IsMain())
   {
      Con::printf("BrowserCefClient: Page loaded (status=%d, url=%s)",
                  httpStatusCode,
                  frame->GetURL().ToString().c_str());
   }
}

void BrowserCefClient::OnLoadError(CefRefPtr<CefBrowser> browser,
                                    CefRefPtr<CefFrame> frame,
                                    ErrorCode errorCode,
                                    const CefString &errorText,
                                    const CefString &failedUrl)
{
   if (frame->IsMain())
   {
      Con::errorf("BrowserCefClient: Load error %d (%s) for %s",
                  errorCode,
                  errorText.ToString().c_str(),
                  failedUrl.ToString().c_str());
   }
}

#endif // TORQUE_CEF_ENABLED
