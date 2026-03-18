//-----------------------------------------------------------------------------
// Browser Render Module for Torque3D
// MIT License - Copyright (c) 2026 Peak AI Design LLC
//-----------------------------------------------------------------------------

#include "browserRender/browserRenderHandler.h"

#ifdef TORQUE_CEF_ENABLED

#ifndef _GFXDEVICE_H_
#include "gfx/gfxDevice.h"
#endif

//-----------------------------------------------------------------------------

BrowserRenderHandler::BrowserRenderHandler(U32 width, U32 height)
   : mWidth(width),
     mHeight(height),
     mDirty(false),
     mPixelBuffer(NULL)
{
   _allocateTexture();
}

BrowserRenderHandler::~BrowserRenderHandler()
{
   if (mPixelBuffer)
   {
      delete[] mPixelBuffer;
      mPixelBuffer = NULL;
   }
   mTexture.free();
}

//-----------------------------------------------------------------------------

void BrowserRenderHandler::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect)
{
   rect = CefRect(0, 0, mWidth, mHeight);
}

//-----------------------------------------------------------------------------

void BrowserRenderHandler::OnPaint(CefRefPtr<CefBrowser> browser,
                                    PaintElementType type,
                                    const RectList &dirtyRects,
                                    const void *buffer,
                                    int width,
                                    int height)
{
   // CEF delivers BGRA pixel data.
   // We lock the dynamic texture and copy the pixels in.

   if (!mTexture.getPointer() || !GFXDevice::get())
      return;

   // If the size changed, reallocate.
   if ((U32)width != mWidth || (U32)height != mHeight)
   {
      mWidth = width;
      mHeight = height;
      _allocateTexture();
   }

   GFXLockedRect *locked = mTexture.lock(0, NULL);
   if (locked && locked->bits)
   {
      const U8 *src = static_cast<const U8*>(buffer);
      U8 *dst = locked->bits;

      // CEF gives us width*4 bytes per row (BGRA).
      // The locked texture may have a different pitch (row stride).
      const U32 srcPitch = width * 4;
      const U32 copyWidth = mGetMin((U32)srcPitch, (U32)locked->pitch);

      for (S32 y = 0; y < height; y++)
      {
         dMemcpy(dst, src, copyWidth);
         src += srcPitch;
         dst += locked->pitch;
      }
   }
   mTexture.unlock(0);

   mDirty = true;
}

//-----------------------------------------------------------------------------

void BrowserRenderHandler::resize(U32 width, U32 height)
{
   if (width == mWidth && height == mHeight)
      return;

   mWidth = width;
   mHeight = height;
   _allocateTexture();
}

//-----------------------------------------------------------------------------

void BrowserRenderHandler::_allocateTexture()
{
   // Free existing resources.
   mTexture.free();
   if (mPixelBuffer)
   {
      delete[] mPixelBuffer;
      mPixelBuffer = NULL;
   }

   if (!GFXDevice::get())
      return;

   // Allocate the CPU-side buffer.
   mPixelBuffer = new U8[mWidth * mHeight * 4];
   dMemset(mPixelBuffer, 0, mWidth * mHeight * 4);

   // Create a dynamic texture (BGRA format to match CEF output).
   mTexture.set(mWidth, mHeight, GFXFormatR8G8B8A8, &GFXDynamicTextureProfile,
                String("BrowserRenderHandler"), 1, 0);
}

#endif // TORQUE_CEF_ENABLED
