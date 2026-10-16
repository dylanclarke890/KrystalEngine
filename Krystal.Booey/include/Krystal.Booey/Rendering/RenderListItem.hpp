#pragma once

#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::html
{
  class HTMLOListElement;
}

namespace krys::boo
{
  class RenderListMarker;

  /// @brief A list item: a block flow with a marker and an ordinal value.
  // FIXME: Painting and the preferred widths (which update the marker's margins and content) come with
  // layout and painting.
  class RenderListItem final : public RenderBlockFlow
  {
    KRYS_NON_COPY_MOVABLE(RenderListItem);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderListItem);

  private:
    WeakPtr<RenderListMarker> _marker;
    mutable Maybe<int32> _value;

  public:
    RenderListItem(dom::Element &element, RenderStyle &&style) noexcept;
    ~RenderListItem() noexcept override;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept final
    {
      return u8"RenderListItem";
    }

    /// @brief The item's ordinal value.
    KRYS_NODISCARD int32 Value() const noexcept
    {
      if (!_value)
      {
        UpdateValueNow();
      }
      return *_value;
    }
    void UpdateValue() noexcept;

    KRYS_NODISCARD utf8_string MarkerTextWithoutSuffix() const noexcept;
    KRYS_NODISCARD utf8_string MarkerTextWithSuffix() const noexcept;

    void UpdateListMarkerNumbers() noexcept;

    static void UpdateItemValuesForOrderedList(const html::HTMLOListElement &list) noexcept;
    KRYS_NODISCARD static uint32 ItemCountForOrderedList(const html::HTMLOListElement &list) noexcept;

    KRYS_NODISCARD RenderStyle ComputeMarkerStyle() const noexcept;

    KRYS_NODISCARD RenderListMarker *MarkerRenderer() const noexcept;
    void SetMarkerRenderer(RenderListMarker &marker) noexcept;

    KRYS_NODISCARD bool IsInReversedOrderedList() const noexcept;

  private:
    void StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept final;

    void UpdateValueNow() const noexcept;
    void CounterDirectivesChanged() noexcept;
  };

  /// @brief Whether @p node is a `<ul>` or `<ol>`.
  KRYS_NODISCARD bool IsHTMLListElement(const dom::Node &node) noexcept;
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderListItem)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderListItem();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
