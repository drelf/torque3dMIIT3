//-----------------------------------------------------------------------------
// Browser Render Module for Torque3D
// MIT License - Copyright (c) 2026 Peak AI Design LLC
//
// CEF off-screen render handler. Receives painted pixel buffers from
// Chromium and copies them into a Torque GFXTexHandle via lock/unlock.
//-----------------------------------------------------------------------------

#ifndef _BROWSER_RENDER_HANDLER_H_
#define _BROWSER_RENDER_HANDLER_H_

#ifdef TORQUE_CEF_ENABLED

#include "include/cef_render_handler.h"

#ifndef _GFXTEXTUREHANDLE_H_
#include "gfx/gfxTextureHandle.h"
#endif
#ifndef _GFXTEXTUREPROFILE_H_
#include "gfx/gfxTextureProfile.h"
#endif
#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif

/// Implements CefRenderHandler for off-screen rendering.
/// CEF paints into a CPU buffer; we copy that buffer into a dynamic
/// GFX texture each frame.
class BrowserRenderHandler : public CefRenderHandler
{
public:
   BrowserRenderHandler(U32 width, U32 height);
   virtual ~BrowserRenderHandler();

   // CefRenderHandler interface
   void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect) override;

   void OnPaint(CefRefPtr<CefBrowser> browser,
                PaintElementType type,
                const RectList &dirtyRects,
                const void *buffer,
                int width,
                int height) override;

   /// Returns the GFX texture that holds the latest browser frame.
   GFXTexHandle& getTexture() { return mTexture; }

   /// Resize the browser viewport.
   void resize(U32 width, U32 height);

   /// Has the texture been updated since we last checked?
   bool isDirty() const { return mDirty; }
   void clearDirty() { mDirty = false; }

private:
   U32 mWidth;
   U32 mHeight;
   bool mDirty;

   /// The dynamic texture we write browser pixels into.
   GFXTexHandle mTexture;

   /// Intermediate CPU buffer that CEF paints to.
   U8 *mPixelBuffer;

   /// (Re)allocate the texture and pixel buffer.
   void _allocateTexture();

   IMPLEMENT_REFCOUNTING(BrowserRenderHandler);
};

#endif // TORQUE_CEF_ENABLED
#endif // _BROWSER_RENDER_HANDLER_H_
