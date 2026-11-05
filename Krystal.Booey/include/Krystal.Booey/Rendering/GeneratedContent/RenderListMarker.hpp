#pragma once

#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/Pair.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::css
{
  class CounterStyle;
}

namespace krys::boo
{
  class RenderListItem;
  class StyleImage;

  /// @brief The text of a list marker, with its suffix.
  struct ListMarkerTextContent
  {
    utf8_string TextWithSuffix;
    // In bytes.
    size_t TextWithoutSuffixLength {0};
    TextDirection TextDirection {TextDirection::LTR};

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return TextWithSuffix.empty();
    }

    KRYS_NODISCARD utf8_stringview TextWithoutSuffix() const noexcept KRYS_LIFETIME_BOUND
    {
      return utf8_stringview {TextWithSuffix}.substr(0, TextWithoutSuffixLength);
    }

    KRYS_NODISCARD utf8_stringview Suffix() const noexcept KRYS_LIFETIME_BOUND
    {
      return utf8_stringview {TextWithSuffix}.substr(TextWithoutSuffixLength);
    }
  };

  /// @brief Used to render the list item's marker.
  /// The RenderListMarker always has to be a child of a RenderListItem.
  class RenderListMarker final : public RenderBox
  {
    KRYS_NON_COPY_MOVABLE(RenderListMarker);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderListMarker);

  private:
    ListMarkerTextContent _textContent;
    RefPtr<StyleImage> _image;

    WeakPtr<RenderListItem> _listItem;
    LayoutUnit _lineLogicalOffsetForListItem;
    Pair<float, float> _layoutBounds;

  public:
    void Paint(PaintInfo &, const LayoutPoint &) noexcept final;

    RenderListMarker(RenderListItem &listItem, RenderStyle &&style) noexcept;
    ~RenderListMarker() noexcept override;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept final
    {
      return u8"RenderListMarker";
    }

    void ImageChanged(WrappedImagePtr, const IntRect * = nullptr) noexcept override;

    KRYS_NODISCARD LayoutRect SelectionRectForRepaint(const RenderLayerModelObject *repaintContainer,
                                                      bool clipToVisibleContent) noexcept final;
    KRYS_NODISCARD bool CanBeSelectionLeaf() const noexcept final
    {
      return true;
    }
    KRYS_NODISCARD LayoutRect LocalSelectionRect() const noexcept;

    KRYS_NODISCARD utf8_string TextWithoutSuffix() const noexcept
    {
      return utf8_string(_textContent.TextWithoutSuffix());
    }
    KRYS_NODISCARD utf8_string TextWithSuffix() const noexcept
    {
      return _textContent.TextWithSuffix;
    }

    KRYS_NODISCARD bool IsInside() const noexcept;
    KRYS_NODISCARD bool IsDisclosureMarker() const noexcept;

    /// @brief Updates the marker's text from its list item's value and its style.
    void UpdateInlineMarginsAndContent() noexcept;

    KRYS_NODISCARD bool IsImage() const noexcept;

    /// @brief Where the marker's image, bullet or text is drawn, relative to the marker.
    KRYS_NODISCARD FloatRect RelativeMarkerRect() noexcept;

    KRYS_NODISCARD LayoutUnit LineLogicalOffsetForListItem() const noexcept
    {
      return _lineLogicalOffsetForListItem;
    }
    KRYS_NODISCARD const RenderListItem *ListItem() const noexcept;

    /// @brief The marker's ascent and descent on its line.
    KRYS_NODISCARD Pair<float, float> LayoutBounds() const noexcept
    {
      return _layoutBounds;
    }

  private:
    void WillBeDestroyed() noexcept final;

    void ComputePreferredLogicalWidths() noexcept final;
    void ComputeIntrinsicLogicalWidths(LayoutUnit &, LayoutUnit &) const noexcept override
    {
      krys_debug_assert(false);
    }
    KRYS_NODISCARD bool CanHaveChildren() const noexcept final
    {
      return false;
    }
    void StyleWillChange(style::Difference diff, const RenderStyle &newStyle) noexcept final;
    void StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept final;
    void Layout() noexcept final;
    KRYS_NODISCARD Pair<float, float> LayoutBoundForTextContent(utf8_stringview text) const noexcept;

    void UpdateInlineMargins() noexcept;
    void UpdateContent() noexcept;
    KRYS_NODISCARD RenderBox *ParentBox(RenderBox &box) noexcept;

    KRYS_NODISCARD RefPtr<css::CounterStyle> GetCounterStyle() const noexcept;
    KRYS_NODISCARD bool WidthUsesMetricsOfPrimaryFont() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderListMarker)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderListMarker();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
