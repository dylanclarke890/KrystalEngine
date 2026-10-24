#pragma once

#include "Krystal.Booey/DOM/DOMRectReadOnly.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "Krystal.Booey/Page/ResizeObserverSize.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo
{
  /// @brief An observed element's new sizes, delivered to its resize observer's callback.
  /// @see https://drafts.csswg.org/resize-observer/#resize-observer-entry-interface
  class ResizeObserverEntry : public RefCounted<ResizeObserverEntry>
  {
  private:
    const Ref<dom::Element> _target;
    const Ref<dom::DOMRectReadOnly> _contentRect;
    // The specification allows several boxes for fragmented elements (multicol), but these lists only ever
    // hold one.
    List<Ref<ResizeObserverSize>> _borderBoxSizes;
    List<Ref<ResizeObserverSize>> _contentBoxSizes;

    ResizeObserverEntry(Ref<dom::Element> &&target, const FloatRect &contentRect, FloatSize borderBoxSize,
                        FloatSize contentBoxSize) noexcept
        : _target(krys::move(target)),
          _contentRect(dom::DOMRectReadOnly::Create(contentRect.X(), contentRect.Y(), contentRect.Width(),
                                                    contentRect.Height())),
          _borderBoxSizes({ResizeObserverSize::Create(borderBoxSize.Width(), borderBoxSize.Height())}),
          _contentBoxSizes({ResizeObserverSize::Create(contentBoxSize.Width(), contentBoxSize.Height())})
    {
    }

  public:
    KRYS_NODISCARD static Ref<ResizeObserverEntry> Create(Ref<dom::Element> &&target,
                                                          const FloatRect &contentRect,
                                                          FloatSize borderBoxSize,
                                                          FloatSize contentBoxSize) noexcept
    {
      return AdoptRef(
        *new ResizeObserverEntry(krys::move(target), contentRect, borderBoxSize, contentBoxSize));
    }

    /// @see https://drafts.csswg.org/resize-observer/#dom-resizeobserverentry-target
    KRYS_NODISCARD dom::Element &Target() const noexcept
    {
      return *_target;
    }

    /// @see https://drafts.csswg.org/resize-observer/#dom-resizeobserverentry-contentrect
    KRYS_NODISCARD dom::DOMRectReadOnly &ContentRect() const noexcept
    {
      return *_contentRect;
    }

    /// @see https://drafts.csswg.org/resize-observer/#dom-resizeobserverentry-borderboxsize
    KRYS_NODISCARD const List<Ref<ResizeObserverSize>> &BorderBoxSize() const noexcept
    {
      return _borderBoxSizes;
    }

    /// @see https://drafts.csswg.org/resize-observer/#dom-resizeobserverentry-contentboxsize
    KRYS_NODISCARD const List<Ref<ResizeObserverSize>> &ContentBoxSize() const noexcept
    {
      return _contentBoxSizes;
    }
  };
}
