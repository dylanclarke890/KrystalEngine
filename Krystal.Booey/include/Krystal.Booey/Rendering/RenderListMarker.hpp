#pragma once

#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"
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
  // FIXME: Layout (the preferred widths, the inline margins, the line offsets and the layout bounds),
  // painting, selection and image loading (the image client) come with layout, painting and resource
  // loading.
  class RenderListMarker final : public RenderBox
  {
    KRYS_NON_COPY_MOVABLE(RenderListMarker);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderListMarker);

  private:
    ListMarkerTextContent _textContent;
    RefPtr<StyleImage> _image;

    WeakPtr<RenderListItem> _listItem;

  public:
    RenderListMarker(RenderListItem &listItem, RenderStyle &&style) noexcept;
    ~RenderListMarker() noexcept override;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept final
    {
      return u8"RenderListMarker";
    }

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

    KRYS_NODISCARD const RenderListItem *ListItem() const noexcept;

  private:
    void WillBeDestroyed() noexcept final;

    // FIXME: The marker's width from its image or text, and its inline margins (updateInlineMargins), once
    // there's inline layout.
    void ComputePreferredLogicalWidths() noexcept final
    {
      krys_debug_assert(NeedsPreferredLogicalWidthsUpdate());
      _minPreferredLogicalWidth = _maxPreferredLogicalWidth = 0_lu;
      ClearNeedsPreferredWidthsUpdate();
    }
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

    void UpdateContent() noexcept;

    KRYS_NODISCARD RefPtr<css::CounterStyle> GetCounterStyle() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderListMarker)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderListMarker();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
