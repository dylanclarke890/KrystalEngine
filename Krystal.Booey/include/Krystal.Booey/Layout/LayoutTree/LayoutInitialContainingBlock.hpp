#pragma once

#include "Krystal.Booey/Layout/LayoutTree/LayoutElementBox.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::layout
{
  /// @brief The root of the layout tree: the rectangle the root element's box lives in.
  class InitialContainingBlock final : public ElementBox
  {
    KRYS_NON_COPY_MOVABLE(InitialContainingBlock);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(InitialContainingBlock);

  public:
    explicit InitialContainingBlock(RenderStyle &&style,
                                    UniquePtr<RenderStyle> &&firstLineStyle = nullptr) noexcept;
    ~InitialContainingBlock() noexcept override = default;

  private:
    const ElementBox &Parent() const = delete;
    const Box *NextSibling() const = delete;
    const Box *NextInFlowSibling() const = delete;
    const Box *NextInFlowOrFloatingSibling() const = delete;
    const Box *PreviousSibling() const = delete;
    const Box *PreviousInFlowSibling() const = delete;
    const Box *PreviousInFlowOrFloatingSibling() const = delete;
    Box *NextSibling() = delete;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::layout::InitialContainingBlock)
  KRYS_NODISCARD static bool IsType(const krys::boo::layout::Box &box) noexcept
  {
    return box.IsInitialContainingBlock();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
