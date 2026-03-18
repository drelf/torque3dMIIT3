//-----------------------------------------------------------------------------
// Browser Render Module for Torque3D
// MIT License - Copyright (c) 2026 Peak AI Design LLC
//-----------------------------------------------------------------------------

#include "browserRender/browserTexture.h"
#include "browserRender/browserManager.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "math/mPoint2.h"

#ifdef TORQUE_CEF_ENABLED
#include "include/cef_app.h"
#include "include/cef_browser.h"
#endif

//-----------------------------------------------------------------------------
// Console object implementation
//-----------------------------------------------------------------------------

IMPLEMENT_CONOBJECT(BrowserTexture);

ConsoleDocClass(BrowserTexture,
   "@brief Renders a web page off-screen and exposes the result as a texture.\n\n"
   "BrowserTexture uses CEF (Chromium Embedded Framework) to render web content "
   "into a dynamic GFX texture. This texture can be bound to any Material's "
   "diffuseMap slot, allowing web pages to appear on 3D surfaces in the world.\n\n"
   "@tsexample\n"
   "// Create a browser texture\n"
   "%browser = new BrowserTexture() {\n"
   "   url = \"https://example.com\";\n"
   "   resolution = \"1024 768\";\n"
   "   refreshRate = 30;\n"
   "};\n"
   "@endtsexample\n"
   "@ingroup BrowserRender\n"
);

//-----------------------------------------------------------------------------

BrowserTexture::BrowserTexture()
   : mUrl("about:blank"),
     mResolution(1024, 768),
     mRefreshRate(30)
{
#ifdef TORQUE_CEF_ENABLED
   mRenderHandler = nullptr;
   mCefClient = nullptr;
#endif
}

BrowserTexture::~BrowserTexture()
{
}

//-----------------------------------------------------------------------------

void BrowserTexture::initPersistFields()
{
   addField("url", TypeString, Offset(mUrl, BrowserTexture),
      "The URL to load when the browser is created.");

   addField("resolution", TypePoint2I, Offset(mResolution, BrowserTexture),
      "The pixel resolution of the browser viewport (e.g. \"1024 768\").");

   addField("refreshRate", TypeS32, Offset(mRefreshRate, BrowserTexture),
      "Target frames per second for browser rendering.");

   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------

bool BrowserTexture::onAdd()
{
   if (!Parent::onAdd())
      return false;

   // Generate a unique named texture target.
   _generateTargetName();

   // Register the named target so materials can find us.
   if (!mNamedTarget.registerWithName(mTargetName))
   {
      Con::errorf("BrowserTexture::onAdd - Failed to register target '%s'", mTargetName.c_str());
      return false;
   }

   // Create the browser.
   _createBrowser();

   // Register with the BrowserManager for per-frame updates.
   BrowserManager::get()->addBrowser(this);

   // Load the initial URL.
   if (mUrl.isNotEmpty() && mUrl != String("about:blank"))
      loadUrl(mUrl.c_str());

   return true;
}

void BrowserTexture::onRemove()
{
   // Unregister from tick updates.
   BrowserManager::get()->removeBrowser(this);

   // Tear down the browser.
   _destroyBrowser();

   // Unregister the named target.
   mNamedTarget.unregister();

   Parent::onRemove();
}

//-----------------------------------------------------------------------------
// Navigation
//-----------------------------------------------------------------------------

void BrowserTexture::loadUrl(const char *url)
{
   mUrl = url;

#ifdef TORQUE_CEF_ENABLED
   if (mCefClient && mCefClient->isValid())
   {
      CefRefPtr<CefBrowser> browser = mCefClient->getBrowser();
      browser->GetMainFrame()->LoadURL(CefString(url));
   }
#endif

   Con::printf("BrowserTexture: Loading URL '%s'", url);
}

void BrowserTexture::executeJavaScript(const char *code)
{
#ifdef TORQUE_CEF_ENABLED
   if (mCefClient && mCefClient->isValid())
   {
      CefRefPtr<CefBrowser> browser = mCefClient->getBrowser();
      browser->GetMainFrame()->ExecuteJavaScript(
         CefString(code), browser->GetMainFrame()->GetURL(), 0);
   }
#endif
}

void BrowserTexture::reload()
{
#ifdef TORQUE_CEF_ENABLED
   if (mCefClient && mCefClient->isValid())
      mCefClient->getBrowser()->Reload();
#endif
}

void BrowserTexture::goBack()
{
#ifdef TORQUE_CEF_ENABLED
   if (mCefClient && mCefClient->isValid())
      mCefClient->getBrowser()->GoBack();
#endif
}

void BrowserTexture::goForward()
{
#ifdef TORQUE_CEF_ENABLED
   if (mCefClient && mCefClient->isValid())
      mCefClient->getBrowser()->GoForward();
#endif
}

//-----------------------------------------------------------------------------
// Resolution
//-----------------------------------------------------------------------------

void BrowserTexture::setResolution(U32 width, U32 height)
{
   mResolution.set(width, height);

#ifdef TORQUE_CEF_ENABLED
   if (mRenderHandler)
      mRenderHandler->resize(width, height);

   if (mCefClient && mCefClient->isValid())
      mCefClient->getBrowser()->GetHost()->WasResized();
#endif
}

//-----------------------------------------------------------------------------
// Input injection
//-----------------------------------------------------------------------------

void BrowserTexture::injectMouseMove(S32 x, S32 y)
{
#ifdef TORQUE_CEF_ENABLED
   if (mCefClient && mCefClient->isValid())
   {
      CefMouseEvent event;
      event.x = x;
      event.y = y;
      event.modifiers = 0;
      mCefClient->getBrowser()->GetHost()->SendMouseMoveEvent(event, false);
   }
#endif
}

void BrowserTexture::injectMouseDown(S32 x, S32 y, BrowserMouseButton button)
{
#ifdef TORQUE_CEF_ENABLED
   if (mCefClient && mCefClient->isValid())
   {
      CefMouseEvent event;
      event.x = x;
      event.y = y;
      event.modifiers = 0;

      CefBrowserHost::MouseButtonType cefBtn = MBT_LEFT;
      if (button == BROWSER_MOUSE_MIDDLE) cefBtn = MBT_MIDDLE;
      else if (button == BROWSER_MOUSE_RIGHT) cefBtn = MBT_RIGHT;

      mCefClient->getBrowser()->GetHost()->SendMouseClickEvent(event, cefBtn, false, 1);
   }
#endif
}

void BrowserTexture::injectMouseUp(S32 x, S32 y, BrowserMouseButton button)
{
#ifdef TORQUE_CEF_ENABLED
   if (mCefClient && mCefClient->isValid())
   {
      CefMouseEvent event;
      event.x = x;
      event.y = y;
      event.modifiers = 0;

      CefBrowserHost::MouseButtonType cefBtn = MBT_LEFT;
      if (button == BROWSER_MOUSE_MIDDLE) cefBtn = MBT_MIDDLE;
      else if (button == BROWSER_MOUSE_RIGHT) cefBtn = MBT_RIGHT;

      mCefClient->getBrowser()->GetHost()->SendMouseClickEvent(event, cefBtn, true, 1);
   }
#endif
}

void BrowserTexture::injectMouseWheel(S32 x, S32 y, S32 deltaX, S32 deltaY)
{
#ifdef TORQUE_CEF_ENABLED
   if (mCefClient && mCefClient->isValid())
   {
      CefMouseEvent event;
      event.x = x;
      event.y = y;
      event.modifiers = 0;
      mCefClient->getBrowser()->GetHost()->SendMouseWheelEvent(event, deltaX, deltaY);
   }
#endif
}

void BrowserTexture::injectKeyDown(U32 keyCode, bool shift, bool ctrl, bool alt)
{
#ifdef TORQUE_CEF_ENABLED
   if (mCefClient && mCefClient->isValid())
   {
      CefKeyEvent event;
      event.type = KEYEVENT_RAWKEYDOWN;
      event.windows_key_code = keyCode;
      event.native_key_code = keyCode;
      event.modifiers = 0;
      if (shift) event.modifiers |= EVENTFLAG_SHIFT_DOWN;
      if (ctrl)  event.modifiers |= EVENTFLAG_CONTROL_DOWN;
      if (alt)   event.modifiers |= EVENTFLAG_ALT_DOWN;
      mCefClient->getBrowser()->GetHost()->SendKeyEvent(event);
   }
#endif
}

void BrowserTexture::injectKeyUp(U32 keyCode, bool shift, bool ctrl, bool alt)
{
#ifdef TORQUE_CEF_ENABLED
   if (mCefClient && mCefClient->isValid())
   {
      CefKeyEvent event;
      event.type = KEYEVENT_KEYUP;
      event.windows_key_code = keyCode;
      event.native_key_code = keyCode;
      event.modifiers = 0;
      if (shift) event.modifiers |= EVENTFLAG_SHIFT_DOWN;
      if (ctrl)  event.modifiers |= EVENTFLAG_CONTROL_DOWN;
      if (alt)   event.modifiers |= EVENTFLAG_ALT_DOWN;
      mCefClient->getBrowser()->GetHost()->SendKeyEvent(event);
   }
#endif
}

void BrowserTexture::injectKeyChar(U32 charCode)
{
#ifdef TORQUE_CEF_ENABLED
   if (mCefClient && mCefClient->isValid())
   {
      CefKeyEvent event;
      event.type = KEYEVENT_CHAR;
      event.character = charCode;
      event.windows_key_code = charCode;
      event.modifiers = 0;
      mCefClient->getBrowser()->GetHost()->SendKeyEvent(event);
   }
#endif
}

//-----------------------------------------------------------------------------
// Texture / target access
//-----------------------------------------------------------------------------

const char* BrowserTexture::getTextureTargetName() const
{
   return mTargetName.c_str();
}

GFXTexHandle BrowserTexture::getTexture() const
{
#ifdef TORQUE_CEF_ENABLED
   if (mRenderHandler)
      return mRenderHandler->getTexture();
#endif
   return GFXTexHandle();
}

bool BrowserTexture::isReady() const
{
#ifdef TORQUE_CEF_ENABLED
   return mCefClient && mCefClient->isValid();
#else
   return false;
#endif
}

const char* BrowserTexture::getCurrentUrl() const
{
#ifdef TORQUE_CEF_ENABLED
   if (mCefClient && mCefClient->isValid())
   {
      static char urlBuf[2048];
      CefString url = mCefClient->getBrowser()->GetMainFrame()->GetURL();
      dStrncpy(urlBuf, url.ToString().c_str(), sizeof(urlBuf) - 1);
      urlBuf[sizeof(urlBuf) - 1] = '\0';
      return urlBuf;
   }
#endif
   return mUrl.c_str();
}

//-----------------------------------------------------------------------------
// Per-frame update
//-----------------------------------------------------------------------------

void BrowserTexture::processTick()
{
#ifdef TORQUE_CEF_ENABLED
   // Update the named texture target with the latest browser frame.
   if (mRenderHandler && mRenderHandler->isDirty())
   {
      mNamedTarget.setTexture(mRenderHandler->getTexture());
      mRenderHandler->clearDirty();
   }
#endif
}

//-----------------------------------------------------------------------------
// Internal
//-----------------------------------------------------------------------------

void BrowserTexture::_generateTargetName()
{
   static U32 sNextId = 0;
   mTargetName = String::ToString("BrowserTex_%d", sNextId++);
}

void BrowserTexture::_createBrowser()
{
#ifdef TORQUE_CEF_ENABLED
   mRenderHandler = new BrowserRenderHandler(mResolution.x, mResolution.y);
   mCefClient = new BrowserCefClient(mRenderHandler);

   // Off-screen browser window info.
   CefWindowInfo windowInfo;
   windowInfo.SetAsWindowless(0);

   // Browser settings.
   CefBrowserSettings settings;
   settings.windowless_frame_rate = mRefreshRate;

   // Create the browser asynchronously. OnAfterCreated() will be called
   // on the CefClient when it is ready.
   CefBrowserHost::CreateBrowser(windowInfo, mCefClient, CefString(mUrl.c_str()),
                                  settings, nullptr, nullptr);

   Con::printf("BrowserTexture: Creating browser '%s' (%dx%d @ %dfps)",
               mTargetName.c_str(), mResolution.x, mResolution.y, mRefreshRate);
#else
   Con::warnf("BrowserTexture: CEF is not enabled in this build.");
#endif
}

void BrowserTexture::_destroyBrowser()
{
#ifdef TORQUE_CEF_ENABLED
   if (mCefClient && mCefClient->isValid())
   {
      mCefClient->getBrowser()->GetHost()->CloseBrowser(true);
   }
   mCefClient = nullptr;
   mRenderHandler = nullptr;
#endif
}

//-----------------------------------------------------------------------------
// Console methods (TorqueScript API)
//-----------------------------------------------------------------------------

DefineEngineMethod(BrowserTexture, loadUrl, void, (const char *url), ,
   "Navigate the browser to the specified URL.\n"
   "@param url The URL to load.\n")
{
   object->loadUrl(url);
}

DefineEngineMethod(BrowserTexture, executeJS, void, (const char *code), ,
   "Execute JavaScript code in the browser.\n"
   "@param code The JavaScript code to execute.\n")
{
   object->executeJavaScript(code);
}

DefineEngineMethod(BrowserTexture, reload, void, (), ,
   "Reload the current page.\n")
{
   object->reload();
}

DefineEngineMethod(BrowserTexture, goBack, void, (), ,
   "Navigate back in browser history.\n")
{
   object->goBack();
}

DefineEngineMethod(BrowserTexture, goForward, void, (), ,
   "Navigate forward in browser history.\n")
{
   object->goForward();
}

DefineEngineMethod(BrowserTexture, setResolution, void, (Point2I res), ,
   "Set the browser viewport resolution.\n"
   "@param res Resolution as \"width height\".\n")
{
   object->setResolution(res.x, res.y);
}

DefineEngineMethod(BrowserTexture, getTextureTarget, const char *, (), ,
   "Get the named texture target name for binding to materials.\n"
   "@return The target name string.\n")
{
   return object->getTextureTargetName();
}

DefineEngineMethod(BrowserTexture, injectMouseMove, void, (S32 x, S32 y), ,
   "Send a mouse move event to the browser.\n")
{
   object->injectMouseMove(x, y);
}

DefineEngineMethod(BrowserTexture, injectMouseDown, void, (S32 x, S32 y, S32 button), (0),
   "Send a mouse button press to the browser.\n"
   "@param button 0=left, 1=middle, 2=right\n")
{
   object->injectMouseDown(x, y, (BrowserMouseButton)button);
}

DefineEngineMethod(BrowserTexture, injectMouseUp, void, (S32 x, S32 y, S32 button), (0),
   "Send a mouse button release to the browser.\n"
   "@param button 0=left, 1=middle, 2=right\n")
{
   object->injectMouseUp(x, y, (BrowserMouseButton)button);
}

DefineEngineMethod(BrowserTexture, injectMouseWheel, void, (S32 x, S32 y, S32 deltaX, S32 deltaY), ,
   "Send a mouse wheel scroll event to the browser.\n")
{
   object->injectMouseWheel(x, y, deltaX, deltaY);
}

DefineEngineMethod(BrowserTexture, injectKeyPress, void, (U32 keyCode), ,
   "Send a key press event to the browser.\n")
{
   object->injectKeyDown(keyCode, false, false, false);
   object->injectKeyChar(keyCode);
   object->injectKeyUp(keyCode, false, false, false);
}

DefineEngineMethod(BrowserTexture, isReady, bool, (), ,
   "Check if the browser is created and ready.\n"
   "@return True if the browser is active.\n")
{
   return object->isReady();
}

DefineEngineMethod(BrowserTexture, getCurrentUrl, const char *, (), ,
   "Get the current URL the browser is displaying.\n"
   "@return The current URL string.\n")
{
   return object->getCurrentUrl();
}
