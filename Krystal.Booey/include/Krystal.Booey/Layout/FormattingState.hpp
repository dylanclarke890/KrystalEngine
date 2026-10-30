#pragma once

#include "Krystal.Booey/Layout/FormattingContexts/FormattingConstraints.hpp"
#include "Krystal.Booey/Layout/LayoutState.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/CheckedPtr.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::layout
{
  class Box;

  /// @brief The state a formatting context keeps across layouts: the intrinsic widths of its boxes and of its
  /// content.
  class FormattingState
  {
    KRYS_NON_COPY_MOVABLE(FormattingState);

  protected:
    enum class Type : uint8
    {
      Block,
      Table,
      Flex
    };

  private:
    const CheckedRef<LayoutState> _layoutState;
    HashMap<const Box *, IntrinsicWidthConstraints> _intrinsicWidthConstraintsForBoxes;
    Maybe<IntrinsicWidthConstraints> _intrinsicWidthConstraints;
    Type _type;

  public:
    void SetIntrinsicWidthConstraintsForBox(const Box &layoutBox,
                                            IntrinsicWidthConstraints intrinsicWidthConstraints) noexcept;
    KRYS_NODISCARD Maybe<IntrinsicWidthConstraints>
      IntrinsicWidthConstraintsForBox(const Box &layoutBox) const noexcept;
    void ClearIntrinsicWidthConstraints(const Box &layoutBox) noexcept;

    void SetIntrinsicWidthConstraints(IntrinsicWidthConstraints intrinsicWidthConstraints) noexcept
    {
      _intrinsicWidthConstraints = intrinsicWidthConstraints;
    }
    KRYS_NODISCARD Maybe<layout::IntrinsicWidthConstraints> GetIntrinsicWidthConstraints() const noexcept
    {
      return _intrinsicWidthConstraints;
    }

    KRYS_NODISCARD bool IsBlockFormattingState() const noexcept
    {
      return _type == Type::Block;
    }
    KRYS_NODISCARD bool IsTableFormattingState() const noexcept
    {
      return _type == Type::Table;
    }
    KRYS_NODISCARD bool IsFlexFormattingState() const noexcept
    {
      return _type == Type::Flex;
    }

    KRYS_NODISCARD LayoutState &GetLayoutState() const noexcept
    {
      return *_layoutState;
    }

    // FIXME: We need to find a way to limit access to mutable geometry.
    BoxGeometry &GetBoxGeometry(const Box &layoutBox) noexcept;

  protected:
    FormattingState(Type type, LayoutState &layoutState) noexcept;
    ~FormattingState() noexcept;
  };
}
