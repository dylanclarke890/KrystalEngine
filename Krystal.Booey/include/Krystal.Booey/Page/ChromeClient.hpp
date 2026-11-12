/*
 * Copyright (C) 2006-2025 Apple, Inc. All rights reserved.
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * Copyright (C) 2012 Samsung Electronics. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/Graphics/Geometry/FloatRect.hpp"
#include "Krystal.Booey/Page/PointerLockController.hpp"
#include "Krystal.Booey/Platform/Choosers/ColorChooser.hpp"
#include "Krystal.Booey/Platform/Choosers/DataListSuggestionPicker.hpp"
#include "Krystal.Booey/Platform/Choosers/DateTimeChooser.hpp"
#include "Krystal.Booey/Platform/Choosers/FileChooser.hpp"
#include "Krystal.Booey/Platform/Choosers/PopupMenu.hpp"
#include "Krystal.Booey/Platform/GraphicsClient.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::html
{
  class HTMLSelectElement;
}

namespace krys::boo::svg
{
  class SVGImageElement;
}

namespace krys::boo::ax
{
  class AccessibilityObject;
  enum class AXNotification : uint8;
  enum class AXTextChange : uint8;
  enum class AccessibilityDetachmentType : uint8;
}

namespace krys::boo
{
  class GraphicsLayer;
  class GraphicsLayerFactory;
  class IntRect;
  class LocalFrame;
  class Color;
  class ColorChooserClient;
  class DataListSuggestionsClient;
  class DateTimeChooserClient;
  class PopupMenuClient;

  /// @brief What a page asks of its host: the window's side of the page, and its graphics.
  // FIXME: The rest of the chrome client: tooltips, cursors, scrolling the host's view, search popups and the
  // other compositing hooks (layer flushes, compositing triggers, display refresh monitors), once there are
  // any.
  class ChromeClient : public GraphicsClient
  {
    KRYS_NON_COPY_MOVABLE(ChromeClient);

  public:
    ChromeClient() noexcept = default;
    virtual ~ChromeClient() noexcept = default;

    /// @brief The page asks to be brought to the front (`window.focus()`); the host decides whether it is.
    virtual void Focus() noexcept
    {
    }

    /// @brief Called when @p rect (in the main frame's view) needs repainting.
    virtual void InvalidateContentsAndRootView(const IntRect &rect) noexcept
    {
      (void)rect;
    }

    /// @brief Asks the client to schedule the page's next rendering update.
    /// @return Whether the client scheduled it; if not, the page waits for the host's next frame.
    virtual bool ScheduleRenderingUpdate() noexcept
    {
      return false;
    }

    /// @brief Whether this is the chrome client of an SVG image's page.
    KRYS_NODISCARD virtual bool IsSVGImageChromeClient() const noexcept
    {
      return false;
    }

    /// @brief Called when the page's accessibility tree posts @p notification for @p object, for the host to
    /// pass on to assistive technology.
    virtual void PostAccessibilityNotification(ax::AccessibilityObject &object,
                                               ax::AXNotification notification) noexcept
    {
      (void)object;
      (void)notification;
    }
    /// @brief Called when @p object leaves the page's accessibility tree (its node or renderer went, or the
    /// tree is being destroyed), for the host to let go of what it made for it.
    virtual void AccessibilityObjectDetached(ax::AccessibilityObject &object,
                                             ax::AccessibilityDetachmentType detachmentType) noexcept
    {
      (void)object;
      (void)detachmentType;
    }
    /// @brief Called when @p text was inserted into or deleted from @p object's text (an editable region's,
    /// or the web area's) at @p offset, a byte index in its UTF-8 text.
    virtual void PostAccessibilityTextChange(ax::AccessibilityObject &object, ax::AXTextChange change,
                                             uint64 offset, utf8_stringview text) noexcept
    {
      (void)object;
      (void)change;
      (void)offset;
      (void)text;
    }

    /// @brief Called when an `<image>` in the page has loaded its image.
    virtual void DidFinishLoadingImageForSVGImage(svg::SVGImageElement &) noexcept
    {
    }

    /// @brief Pass nullptr as the GraphicsLayer to detach the root layer.
    virtual void AttachRootGraphicsLayer(LocalFrame &, GraphicsLayer *) noexcept = 0;

    /// @brief The factory the page's graphics layers come from; null when the host doesn't composite, so the
    /// page isn't composited.
    KRYS_NODISCARD virtual GraphicsLayerFactory *GetGraphicsLayerFactory() const noexcept
    {
      return nullptr;
    }

    /// @brief Makes an image buffer with the host's own backend; null when the host has none for @p mode, so
    /// the buffer gets one of the page's (a display list).
    KRYS_NODISCARD RefPtr<ImageBuffer>
      CreateImageBuffer(const FloatSize &size, RenderingMode mode, RenderingPurpose purpose,
                        float resolutionScale,
                        const DestinationColorSpace &colorSpace) const noexcept override
    {
      (void)size;
      (void)mode;
      (void)purpose;
      (void)resolutionScale;
      (void)colorSpace;
      return nullptr;
    }

    /// @brief Whether a menu list's items are shown left to right regardless of their direction.
    KRYS_NODISCARD virtual bool SelectItemWritingDirectionIsNatural() const noexcept
    {
      return false;
    }

    /// @brief Whether a menu list's text is aligned in its selected option's direction.
    KRYS_NODISCARD virtual bool SelectItemAlignmentFollowsMenuWritingDirection() const noexcept
    {
      return true;
    }

    /// @brief The options of the focused select changed, for a host showing them in its own picker.
    virtual void FocusedSelectElementDidChangeOptions(const html::HTMLSelectElement &) noexcept
    {
    }

    /// @brief Makes the popup menu a menu list shows its options in; a host that doesn't override this has no
    /// popups, so menu lists don't open.
    KRYS_NODISCARD virtual RefPtr<PopupMenu> CreatePopupMenu(PopupMenuClient &client) const noexcept
    {
      (void)client;
      return nullptr;
    }

    /// @brief Makes and shows the picker a colour input chooses its colour in, with @p initialColor chosen; a
    /// host that doesn't override this has no colour pickers.
    KRYS_NODISCARD virtual RefPtr<ColorChooser> CreateColorChooser(ColorChooserClient &client,
                                                                   const Color &initialColor) noexcept
    {
      (void)client;
      (void)initialColor;
      return nullptr;
    }

    /// @brief Makes the list a text field shows its datalist suggestions in; a host that doesn't override
    /// this shows no suggestions.
    KRYS_NODISCARD virtual RefPtr<DataListSuggestionPicker>
      CreateDataListSuggestionPicker(DataListSuggestionsClient &client) noexcept
    {
      (void)client;
      return nullptr;
    }

    /// @brief Whether the host's suggestion list shows the suggestions' labels, so typing matches them too.
    KRYS_NODISCARD virtual bool CanShowDataListSuggestionLabels() const noexcept
    {
      return false;
    }

    /// @brief Makes the picker a date input chooses its date in; a host that doesn't override this has no
    /// date pickers.
    KRYS_NODISCARD virtual RefPtr<DateTimeChooser>
      CreateDateTimeChooser(DateTimeChooserClient &client) noexcept
    {
      (void)client;
      return nullptr;
    }

    /// @brief Shows @p message, a constraint the control @p anchor doesn't satisfy, near it (a validation
    /// bubble), in place of any shown; the host may hide it after a while. A host that doesn't override this
    /// shows no messages.
    virtual void ShowValidationMessage(dom::Element &anchor, const dom::DOMString &message) noexcept
    {
      (void)anchor;
      (void)message;
    }

    /// @brief Hides the validation message shown for @p anchor, if it's shown.
    virtual void HideValidationMessage(const dom::Element &anchor) noexcept
    {
      (void)anchor;
    }

    /// @brief Hides any validation message shown.
    virtual void HideAnyValidationMessage() noexcept
    {
    }

    /// @brief Whether the validation message for @p anchor is shown.
    KRYS_NODISCARD virtual bool IsValidationMessageVisible(const dom::Element &anchor) const noexcept
    {
      (void)anchor;
      return false;
    }

    /// @brief Shows the host's file picker for a file input in @p frame, answering @p chooser with the files
    /// the user chooses, or cancelling it, now or later; a host that doesn't override this has no file
    /// picker, so the choice is cancelled at once.
    virtual void RunOpenPanel(LocalFrame &frame, FileChooser &chooser) noexcept
    {
      (void)frame;
      chooser.Cancel();
    }

    /// @brief Whether the system has a mouse, whose movement the host can report before the system's
    /// acceleration (pointer lock's unadjusted movement).
    KRYS_NODISCARD virtual bool HasAccessoryMousePointingDevice() const noexcept
    {
      return false;
    }

    /// @brief The page asks to lock the pointer: the host hides the cursor, keeps it where it is and reports
    /// the mouse's movement, then calls @p completionHandler with whether it did; a host that doesn't
    /// override this has no pointer lock.
    virtual void RequestPointerLock(Func<void(PointerLockRequestResult)> &&completionHandler) noexcept
    {
      completionHandler(PointerLockRequestResult::Unsupported);
    }

    /// @brief The page asks to unlock the pointer: the host shows the cursor again, then calls
    /// @p completionHandler with whether it did.
    virtual void RequestPointerUnlock(Func<void(bool)> &&completionHandler) noexcept
    {
      completionHandler(false);
    }
  };

  /// @brief The chrome client of a page that has no host.
  class EmptyChromeClient final : public ChromeClient
  {
  public:
    void AttachRootGraphicsLayer(LocalFrame &, GraphicsLayer *) noexcept final
    {
    }
  };
}
