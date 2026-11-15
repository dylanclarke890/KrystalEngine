#pragma once

#include "Krystal.Core/Types/String.hpp"

namespace krys::boo::EventNames
{
  constexpr const char8 *ReadyStateChange = u8"readystatechange";
  constexpr const char8 *Abort = u8"abort";
  constexpr const char8 *Change = u8"change";
  constexpr const char8 *Invalid = u8"invalid";
  constexpr const char8 *Submit = u8"submit";
  constexpr const char8 *HashChange = u8"hashchange";
  constexpr const char8 *Reset = u8"reset";
  constexpr const char8 *FormData = u8"formdata";
  constexpr const char8 *Select = u8"select";
  constexpr const char8 *Load = u8"load";
  constexpr const char8 *BeginEvent = u8"beginEvent";
  constexpr const char8 *EndEvent = u8"endEvent";
  constexpr const char8 *RepeatEvent = u8"repeatEvent";
  constexpr const char8 *DOMContentLoaded = u8"DOMContentLoaded";
  constexpr const char8 *PageShow = u8"pageshow";
  constexpr const char8 *PopState = u8"popstate";
  constexpr const char8 *Navigate = u8"navigate";
  constexpr const char8 *NavigateSuccess = u8"navigatesuccess";
  constexpr const char8 *NavigateError = u8"navigateerror";
  constexpr const char8 *CurrentEntryChange = u8"currententrychange";
  constexpr const char8 *Dispose = u8"dispose";
  constexpr const char8 *PageHide = u8"pagehide";
  constexpr const char8 *Unload = u8"unload";
  constexpr const char8 *Error = u8"error";
  constexpr const char8 *Click = u8"click";
  constexpr const char8 *AuxClick = u8"auxclick";
  constexpr const char8 *DblClick = u8"dblclick";
  constexpr const char8 *ContextMenu = u8"contextmenu";
  constexpr const char8 *MouseDown = u8"mousedown";
  constexpr const char8 *MouseUp = u8"mouseup";
  constexpr const char8 *MouseMove = u8"mousemove";
  constexpr const char8 *MouseOver = u8"mouseover";
  constexpr const char8 *MouseOut = u8"mouseout";
  constexpr const char8 *MouseEnter = u8"mouseenter";
  constexpr const char8 *MouseLeave = u8"mouseleave";
  constexpr const char8 *PointerDown = u8"pointerdown";
  constexpr const char8 *PointerUp = u8"pointerup";
  constexpr const char8 *PointerMove = u8"pointermove";
  constexpr const char8 *PointerRawUpdate = u8"pointerrawupdate";
  constexpr const char8 *PointerCancel = u8"pointercancel";
  constexpr const char8 *PointerOver = u8"pointerover";
  constexpr const char8 *PointerOut = u8"pointerout";
  constexpr const char8 *PointerEnter = u8"pointerenter";
  constexpr const char8 *PointerLeave = u8"pointerleave";
  constexpr const char8 *GotPointerCapture = u8"gotpointercapture";
  constexpr const char8 *LostPointerCapture = u8"lostpointercapture";
  constexpr const char8 *PointerLockChange = u8"pointerlockchange";
  constexpr const char8 *PointerLockError = u8"pointerlockerror";
  constexpr const char8 *KeyDown = u8"keydown";
  constexpr const char8 *KeyPress = u8"keypress";
  constexpr const char8 *KeyUp = u8"keyup";
  constexpr const char8 *Resize = u8"resize";
  constexpr const char8 *Scroll = u8"scroll";
  constexpr const char8 *ScrollEnd = u8"scrollend";
  constexpr const char8 *BeforeCopy = u8"beforecopy";
  constexpr const char8 *Copy = u8"copy";
  constexpr const char8 *BeforeCut = u8"beforecut";
  constexpr const char8 *Cut = u8"cut";
  constexpr const char8 *BeforePaste = u8"beforepaste";
  constexpr const char8 *Paste = u8"paste";
  constexpr const char8 *DragStart = u8"dragstart";
  constexpr const char8 *Drag = u8"drag";
  constexpr const char8 *DragEnter = u8"dragenter";
  constexpr const char8 *DragOver = u8"dragover";
  constexpr const char8 *DragLeave = u8"dragleave";
  constexpr const char8 *Drop = u8"drop";
  constexpr const char8 *DragEnd = u8"dragend";
  constexpr const char8 *BeforeInput = u8"beforeinput";
  constexpr const char8 *Input = u8"input";
  constexpr const char8 *TextInput = u8"textInput";
  constexpr const char8 *CompositionStart = u8"compositionstart";
  constexpr const char8 *CompositionUpdate = u8"compositionupdate";
  constexpr const char8 *CompositionEnd = u8"compositionend";
  constexpr const char8 *Focus = u8"focus";
  constexpr const char8 *Blur = u8"blur";
  constexpr const char8 *WebkitBeforeTextInserted = u8"webkitBeforeTextInserted";
  constexpr const char8 *SelectStart = u8"selectstart";
  constexpr const char8 *SelectionChange = u8"selectionchange";
  constexpr const char8 *VisibilityChange = u8"visibilitychange";
  constexpr const char8 *SlotChange = u8"slotchange";
  constexpr const char8 *TouchStart = u8"touchstart";
  constexpr const char8 *TouchMove = u8"touchmove";
  constexpr const char8 *TouchEnd = u8"touchend";
  constexpr const char8 *TouchCancel = u8"touchcancel";
  constexpr const char8 *Wheel = u8"wheel";
  constexpr const char8 *MouseWheel = u8"mousewheel";
  constexpr const char8 *AnimationCancel = u8"animationcancel";
  constexpr const char8 *AnimationEnd = u8"animationend";
  constexpr const char8 *AnimationIteration = u8"animationiteration";
  constexpr const char8 *AnimationStart = u8"animationstart";
  constexpr const char8 *TransitionCancel = u8"transitioncancel";
  constexpr const char8 *TransitionEnd = u8"transitionend";
  constexpr const char8 *TransitionRun = u8"transitionrun";
  constexpr const char8 *TransitionStart = u8"transitionstart";
  constexpr const char8 *Cancel = u8"cancel";
  constexpr const char8 *Close = u8"close";
  constexpr const char8 *BeforeToggle = u8"beforetoggle";
  constexpr const char8 *Toggle = u8"toggle";
  constexpr const char8 *BeforeMatch = u8"beforematch";
  constexpr const char8 *Command = u8"command";
  constexpr const char8 *ContentVisibilityAutoStateChange = u8"contentvisibilityautostatechange";
  constexpr const char8 *Finish = u8"finish";
  constexpr const char8 *Remove = u8"remove";

  namespace Legacy
  {
    constexpr const char8 *AnimationEnd = u8"webkitAnimationEnd";
    constexpr const char8 *AnimationIteration = u8"webkitAnimationIteration";
    constexpr const char8 *AnimationStart = u8"webkitAnimationStart";
    constexpr const char8 *TransitionEnd = u8"webkitTransitionEnd";
  }
}