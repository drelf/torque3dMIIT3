//-----------------------------------------------------------------------------
// Browser Render Module for Torque3D
// MIT License - Copyright (c) 2026 Peak AI Design LLC
//
// BrowserTexture - A SimObject that owns a CEF off-screen browser and
// exposes its rendered output as a GFX texture. This texture can be
// bound to any Material diffuseMap slot, allowing web content to appear
// on 3D surfaces.
//
// TorqueScript usage:
//   %browser = new BrowserTexture() {
//      url = "https://example.com";
//      resolution = "1024 768";
//      refreshRate = 30;
//   };
//   // Bind to a material:
//   MyScreenMaterial.diffuseMap[0] = %browser.getTextureTargetName();
//-----------------------------------------------------------------------------

#ifndef _BROWSER_TEXTURE_H_
#define _BROWSER_TEXTURE_H_

#ifndef _SIMOBJECT_H_
#include "console/simObject.h"
#endif
#ifndef _GFXTEXTUREHANDLE_H_
#include "gfx/gfxTextureHandle.h"
#endif
#ifndef _MATTEXTURETARGET_H_
#include "materials/matTextureTarget.h"
#endif
#ifndef _BROWSER_TYPES_H_
#include "browserRender/browserTypes.h"
#endif

#ifdef TORQUE_CEF_ENABLED
#include "browserRender/browserRenderHandler.h"
#include "browserRender/browserClient.h"
#endif

/// A script-accessible object that manages an off-screen browser instance
/// and provides its rendered content as a named texture target.
class BrowserTexture : public SimObject
{
   typedef SimObject Parent;

public:
   DECLARE_CONOBJECT(BrowserTexture);

   BrowserTexture();
   virtual ~BrowserTexture();

   static void initPersistFields();

   bool onAdd() override;
   void onRemove() override;

   /// Navigate to a URL.
   void loadUrl(const char *url);

   /// Execute JavaScript in the browser.
   void executeJavaScript(const char *code);

   /// Reload the current page.
   void reload();

   /// Go back/forward in history.
   void goBack();
   void goForward();

   /// Resize the browser viewport.
   void setResolution(U32 width, U32 height);

   /// Send an input event to the browser.
   void injectMouseMove(S32 x, S32 y);
   void injectMouseDown(S32 x, S32 y, BrowserMouseButton button);
   void injectMouseUp(S32 x, S32 y, BrowserMouseButton button);
   void injectMouseWheel(S32 x, S32 y, S32 deltaX, S32 deltaY);
   void injectKeyDown(U32 keyCode, bool shift, bool ctrl, bool alt);
   void injectKeyUp(U32 keyCode, bool shift, bool ctrl, bool alt);
   void injectKeyChar(U32 charCode);

   /// Get the named texture target name for material binding.
   const char* getTextureTargetName() const;

   /// Get the underlying GFX texture (for C++ consumers).
   GFXTexHandle getTexture() const;

   /// Called each frame to pump CEF message loop.
   void processTick();

   /// Is the browser ready and rendering?
   bool isReady() const;

   /// Get current URL.
   const char* getCurrentUrl() const;

protected:
   /// The URL to load when the object is added.
   String mUrl;

   /// Browser viewport resolution.
   Point2I mResolution;

   /// Target frame rate for the browser.
   U32 mRefreshRate;

   /// Named texture target so materials can reference us.
   NamedTexTarget mNamedTarget;

   /// Unique target name for this instance.
   String mTargetName;

#ifdef TORQUE_CEF_ENABLED
   /// CEF render handler (receives pixels).
   CefRefPtr<BrowserRenderHandler> mRenderHandler;

   /// CEF client (owns the browser).
   CefRefPtr<BrowserCefClient> mCefClient;
#endif

   /// Generate a unique target name for this browser instance.
   void _generateTargetName();

   /// Create the CEF browser instance.
   void _createBrowser();

   /// Destroy the CEF browser instance.
   void _destroyBrowser();
};

#endif // _BROWSER_TEXTURE_H_
