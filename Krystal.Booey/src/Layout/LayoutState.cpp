#include "Krystal.Booey/Layout/LayoutState.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Block/BlockFormattingState.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutInitialContainingBlock.hpp"

namespace krys::boo::layout
{
  LayoutState::LayoutState(const dom::Document &document, const ElementBox &rootContainer, Type type,
                           FormattingContextLayoutFunction &&formattingContextLayoutFunction,
                           FormattingContextLogicalWidthFunction &&formattingContextLogicalWidthFunction,
                           FormattingContextLogicalHeightFunction &&formattingContextLogicalHeightFunction,
                           FormattingContextLayoutForBlockInInlineFunction
                             &&formattingContextLayoutForBlockInInlineFunction) noexcept
      : _type(type), _rootContainer(ShareCheckedRef(rootContainer)),
        _formattingContextLayoutFunction(krys::move(formattingContextLayoutFunction)),
        _formattingContextLogicalWidthFunction(krys::move(formattingContextLogicalWidthFunction)),
        _formattingContextLogicalHeightFunction(krys::move(formattingContextLogicalHeightFunction)),
        _formattingContextLayoutForBlockInInlineFunction(
          krys::move(formattingContextLayoutForBlockInInlineFunction))
  {
    // It makes absolutely no sense to construct a dedicated layout state for a non-formatting context root
    // (layout would be a no-op).
    krys_debug_assert(Root().EstablishesFormattingContext());
    UpdateQuirksMode(document);
  }

  LayoutState::~LayoutState() noexcept = default;

  void LayoutState::UpdateQuirksMode(const dom::Document &document) noexcept
  {
    auto quirksMode = [&]
    {
      if (document.InLimitedQuirksMode())
      {
        return LayoutState::QuirksMode::Limited;
      }
      if (document.InQuirksMode())
      {
        return LayoutState::QuirksMode::Yes;
      }
      return LayoutState::QuirksMode::No;
    };
    SetQuirksMode(quirksMode());
  }

  BoxGeometry &LayoutState::GeometryForRootBox() noexcept
  {
    return EnsureGeometryForBox(Root());
  }

  BoxGeometry &LayoutState::EnsureGeometryForBoxSlow(const Box &layoutBox) noexcept
  {
    if (_type == Type::Primary) KRYS_LIKELY
    {
#if KRYS_DEBUG
      krys_debug_assert(!layoutBox._cachedGeometryForPrimaryLayoutState);
      krys_debug_assert(!layoutBox._primaryLayoutState);
      layoutBox._primaryLayoutState = CreateWeakPtr(this);
#endif
      layoutBox._cachedGeometryForPrimaryLayoutState = CreateUnique<BoxGeometry>();
      return *layoutBox._cachedGeometryForPrimaryLayoutState;
    }

    auto &boxGeometry = _layoutBoxToBoxGeometry[&layoutBox];
    if (!boxGeometry)
    {
      boxGeometry = CreateUnique<BoxGeometry>();
    }
    return *boxGeometry;
  }

  bool LayoutState::HasFormattingState(const ElementBox &formattingContextRoot) const noexcept
  {
    krys_debug_assert(formattingContextRoot.EstablishesFormattingContext());
    return _blockFormattingStates.contains(&formattingContextRoot);
  }

  FormattingState &
    LayoutState::FormattingStateForFormattingContext(const ElementBox &formattingContextRoot) const noexcept
  {
    krys_debug_assert(formattingContextRoot.EstablishesFormattingContext());

    if (formattingContextRoot.EstablishesBlockFormattingContext())
    {
      return FormattingStateForBlockFormattingContext(formattingContextRoot);
    }

    // FIXME: The table formatting state, once there's a table formatting context.
    krys_unreachable();
  }

  BlockFormattingState &LayoutState::FormattingStateForBlockFormattingContext(
    const ElementBox &blockFormattingContextRoot) const noexcept
  {
    krys_debug_assert(blockFormattingContextRoot.EstablishesBlockFormattingContext());
    return *_blockFormattingStates.at(&blockFormattingContextRoot);
  }

  BlockFormattingState &
    LayoutState::EnsureBlockFormattingState(const ElementBox &formattingContextRoot) noexcept
  {
    krys_debug_assert(formattingContextRoot.EstablishesBlockFormattingContext());

    auto &formattingState = _blockFormattingStates[&formattingContextRoot];
    if (!formattingState)
    {
      formattingState = CreateUnique<BlockFormattingState>(*this, formattingContextRoot);
    }
    return *formattingState;
  }

  void LayoutState::DestroyBlockFormattingState(const ElementBox &formattingContextRoot) noexcept
  {
    krys_debug_assert(formattingContextRoot.EstablishesBlockFormattingContext());
    _blockFormattingStates.erase(&formattingContextRoot);
  }

  void LayoutState::LayoutWithFormattingContextForBox(const ElementBox &box,
                                                      Maybe<LayoutUnit> widthConstraint,
                                                      Maybe<LayoutUnit> heightConstraint) const noexcept
  {
    auto &layoutState = const_cast<LayoutState &>(*this);
    layoutState._formattingContextLayoutFunction(box, widthConstraint, heightConstraint, layoutState);
  }

  LayoutUnit LayoutState::LogicalWidthWithFormattingContextForBox(
    const ElementBox &box, layout_integration::LogicalWidthType logicalWidthType) const noexcept
  {
    return const_cast<LayoutState &>(*this)._formattingContextLogicalWidthFunction(box, logicalWidthType);
  }

  LayoutUnit LayoutState::LogicalHeightWithFormattingContextForBox(
    const ElementBox &box, layout_integration::LogicalHeightType logicalHeightType) const noexcept
  {
    return const_cast<LayoutState &>(*this)._formattingContextLogicalHeightFunction(box, logicalHeightType);
  }

  void LayoutState::LayoutWithFormattingContextForBlockInInline(
    const ElementBox &block, LayoutPoint blockLineLogicalTopLeft,
    const InlineLayoutState &inlineLayoutState) const noexcept
  {
    auto &layoutState = const_cast<LayoutState &>(*this);
    layoutState._formattingContextLayoutForBlockInInlineFunction(
      block, blockLineLogicalTopLeft, const_cast<InlineLayoutState &>(inlineLayoutState), layoutState);
  }
}
