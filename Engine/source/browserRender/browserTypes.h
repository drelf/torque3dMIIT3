//-----------------------------------------------------------------------------
// Browser Render Module for Torque3D
// MIT License - Copyright (c) 2026 Peak AI Design LLC
//-----------------------------------------------------------------------------

#ifndef _BROWSER_TYPES_H_
#define _BROWSER_TYPES_H_

#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif

/// Mouse button identifiers for browser input.
enum BrowserMouseButton
{
   BROWSER_MOUSE_LEFT = 0,
   BROWSER_MOUSE_MIDDLE,
   BROWSER_MOUSE_RIGHT
};

/// Browser input event types.
enum BrowserInputEventType
{
   BROWSER_EVENT_MOUSE_MOVE = 0,
   BROWSER_EVENT_MOUSE_DOWN,
   BROWSER_EVENT_MOUSE_UP,
   BROWSER_EVENT_MOUSE_WHEEL,
   BROWSER_EVENT_KEY_DOWN,
   BROWSER_EVENT_KEY_UP,
   BROWSER_EVENT_KEY_CHAR
};

/// Encapsulates a single input event to be forwarded to the browser.
struct BrowserInputEvent
{
   BrowserInputEventType type;

   // Mouse data
   S32 mouseX;
   S32 mouseY;
   S32 scrollDeltaX;
   S32 scrollDeltaY;
   BrowserMouseButton mouseButton;

   // Keyboard data
   U32 keyCode;
   U32 charCode;
   bool shiftDown;
   bool ctrlDown;
   bool altDown;

   BrowserInputEvent()
      : type(BROWSER_EVENT_MOUSE_MOVE),
        mouseX(0), mouseY(0),
        scrollDeltaX(0), scrollDeltaY(0),
        mouseButton(BROWSER_MOUSE_LEFT),
        keyCode(0), charCode(0),
        shiftDown(false), ctrlDown(false), altDown(false)
   {
   }
};

#endif // _BROWSER_TYPES_H_
