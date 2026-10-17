#pragma once

#include "Krystal.Booey/Graphics/LayoutPoint.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxGeometry.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutElementBox.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeCheckedPtr.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/CheckedPtr.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo::layout_integration
{
  enum class LogicalWidthType : uint8;
  enum class LogicalHeightType : uint8;
}

namespace krys::boo::layout
{
  class BlockFormattingState;
  class FormattingState;
  class InlineContentCache;
  class InlineLayoutState;

  /// @brief The geometry of the layout boxes, and the formatting states of their formatting contexts.
  // FIXME: The table formatting states, once there's a table formatting context.
  class LayoutState : public CanMakeWeakPtr<LayoutState>, public CanMakeCheckedPtr<LayoutState>
  {
    KRYS_NON_COPY_MOVABLE(LayoutState);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(LayoutState);

  public:
    // Primary layout state has a direct geometry cache in layout boxes.
    enum class Type : uint8
    {
      Primary,
      Secondary
    };

    using FormattingContextLayoutFunction =
      Func<void(const ElementBox &, Maybe<LayoutUnit>, Maybe<LayoutUnit>, LayoutState &)>;
    using FormattingContextLogicalWidthFunction =
      Func<LayoutUnit(const ElementBox &, layout_integration::LogicalWidthType)>;
    using FormattingContextLogicalHeightFunction =
      Func<LayoutUnit(const ElementBox &, layout_integration::LogicalHeightType)>;
    using FormattingContextLayoutForBlockInInlineFunction =
      Func<void(const ElementBox &, LayoutPoint, InlineLayoutState &, LayoutState &)>;

    enum class QuirksMode : uint8
    {
      No,
      Limited,
      Yes
    };

  private:
    const Type _type;

    HashMap<const ElementBox *, UniquePtr<BlockFormattingState>> _blockFormattingStates;
    HashMap<const ElementBox *, UniquePtr<InlineContentCache>> _inlineContentCaches;

    HashMap<const Box *, UniquePtr<BoxGeometry>> _layoutBoxToBoxGeometry;
    QuirksMode _quirksMode {QuirksMode::No};

    const CheckedRef<const ElementBox> _rootContainer;

    FormattingContextLayoutFunction _formattingContextLayoutFunction;
    FormattingContextLogicalWidthFunction _formattingContextLogicalWidthFunction;
    FormattingContextLogicalHeightFunction _formattingContextLogicalHeightFunction;
    FormattingContextLayoutForBlockInInlineFunction _formattingContextLayoutForBlockInInlineFunction;

  public:
    LayoutState(const dom::Document &document, const ElementBox &rootContainer, Type type,
                FormattingContextLayoutFunction &&formattingContextLayoutFunction,
                FormattingContextLogicalWidthFunction &&formattingContextLogicalWidthFunction,
                FormattingContextLogicalHeightFunction &&formattingContextLogicalHeightFunction,
                FormattingContextLayoutForBlockInInlineFunction
                  &&formattingContextLayoutForBlockInInlineFunction) noexcept;
    ~LayoutState() noexcept;

    KRYS_NODISCARD Type GetType() const noexcept
    {
      return _type;
    }

    void UpdateQuirksMode(const dom::Document &document) noexcept;

    KRYS_NODISCARD InlineContentCache &
      GetInlineContentCache(const ElementBox &formattingContextRoot) noexcept;

    BlockFormattingState &EnsureBlockFormattingState(const ElementBox &formattingContextRoot) noexcept;

    KRYS_NODISCARD BlockFormattingState &
      FormattingStateForBlockFormattingContext(const ElementBox &blockFormattingContextRoot) const noexcept;

    KRYS_NODISCARD FormattingState &
      FormattingStateForFormattingContext(const ElementBox &formattingRoot) const noexcept;

    void DestroyBlockFormattingState(const ElementBox &formattingContextRoot) noexcept;
    void DestroyInlineContentCache(const ElementBox &formattingContextRoot) noexcept;

    KRYS_NODISCARD bool HasFormattingState(const ElementBox &formattingRoot) const noexcept;

    BoxGeometry &GeometryForRootBox() noexcept;
    BoxGeometry &EnsureGeometryForBox(const Box &layoutBox) noexcept;
    KRYS_NODISCARD const BoxGeometry &GeometryForBox(const Box &layoutBox) const noexcept;

    KRYS_NODISCARD bool HasBoxGeometry(const Box &layoutBox) const noexcept;

    KRYS_NODISCARD bool InQuirksMode() const noexcept
    {
      return _quirksMode == QuirksMode::Yes;
    }
    KRYS_NODISCARD bool InLimitedQuirksMode() const noexcept
    {
      return _quirksMode == QuirksMode::Limited;
    }
    KRYS_NODISCARD bool InStandardsMode() const noexcept
    {
      return _quirksMode == QuirksMode::No;
    }

    KRYS_NODISCARD bool IsTextShapingAcrossInlineBoxesEnabled() const noexcept
    {
      return true;
    }

    KRYS_NODISCARD const ElementBox &Root() const noexcept
    {
      return *_rootContainer;
    }

    // A height constraint is primarily given by Grid Formatting Contexts since it is able to determine
    // one for grid items in many cases which drives the use of setOverridingBorderBoxLogicalHeight.
    void LayoutWithFormattingContextForBox(const ElementBox &box, Maybe<LayoutUnit> widthConstraint,
                                           Maybe<LayoutUnit> heightConstraint) const noexcept;
    KRYS_NODISCARD LayoutUnit LogicalWidthWithFormattingContextForBox(
      const ElementBox &box, layout_integration::LogicalWidthType logicalWidthType) const noexcept;
    KRYS_NODISCARD LayoutUnit LogicalHeightWithFormattingContextForBox(
      const ElementBox &box, layout_integration::LogicalHeightType logicalHeightType) const noexcept;

    void
      LayoutWithFormattingContextForBlockInInline(const ElementBox &block,
                                                  LayoutPoint blockLineLogicalTopLeft,
                                                  const InlineLayoutState &inlineLayoutState) const noexcept;

  private:
    void SetQuirksMode(QuirksMode quirksMode) noexcept
    {
      _quirksMode = quirksMode;
    }

    BoxGeometry &EnsureGeometryForBoxSlow(const Box &layoutBox) noexcept;
  };

  inline bool LayoutState::HasBoxGeometry(const Box &layoutBox) const noexcept
  {
    if (_type == Type::Primary) KRYS_LIKELY
    {
      return layoutBox._cachedGeometryForPrimaryLayoutState != nullptr;
    }
    return _layoutBoxToBoxGeometry.contains(&layoutBox);
  }

  inline BoxGeometry &LayoutState::EnsureGeometryForBox(const Box &layoutBox) noexcept
  {
    if (_type == Type::Primary) KRYS_LIKELY
    {
      if (auto *boxGeometry = layoutBox._cachedGeometryForPrimaryLayoutState.get())
      {
        krys_debug_assert(layoutBox._primaryLayoutState.get() == this);
        return *boxGeometry;
      }
    }
    return EnsureGeometryForBoxSlow(layoutBox);
  }

  inline const BoxGeometry &LayoutState::GeometryForBox(const Box &layoutBox) const noexcept
  {
    if (_type == Type::Primary) KRYS_LIKELY
    {
      krys_debug_assert(layoutBox._primaryLayoutState.get() == this);
      return *layoutBox._cachedGeometryForPrimaryLayoutState;
    }
    krys_debug_assert(layoutBox._primaryLayoutState.get() != this);
    krys_debug_assert(_layoutBoxToBoxGeometry.contains(&layoutBox));
    return *_layoutBoxToBoxGeometry.at(&layoutBox);
  }
}
