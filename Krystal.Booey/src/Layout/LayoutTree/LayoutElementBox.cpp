#include "Krystal.Booey/Layout/LayoutTree/LayoutElementBox.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include <utility>

namespace krys::boo::layout
{
  ElementBox::ElementBox(ElementAttributes &&attributes, RenderStyle &&style,
                         UniquePtr<RenderStyle> &&firstLineStyle,
                         EnumSet<BaseTypeFlag> baseTypeFlags) noexcept
      : Box(krys::move(attributes), krys::move(style), krys::move(firstLineStyle),
            baseTypeFlags | ElementBoxFlag)
  {
  }

  ElementBox::ElementBox(ElementAttributes &&attributes, EnumSet<ListMarkerAttribute> listMarkerAttributes,
                         RenderStyle &&style, UniquePtr<RenderStyle> &&firstLineStyle) noexcept
      : Box(krys::move(attributes), krys::move(style), krys::move(firstLineStyle), ElementBoxFlag),
        _replacedData(CreateUnique<ReplacedData>())
  {
    krys_debug_assert(IsListMarkerBox());
    _replacedData->ListMarkerAttributes = listMarkerAttributes;
  }

  ElementBox::ElementBox(ElementAttributes &&attributes, ReplacedAttributes &&replacedAttributes,
                         RenderStyle &&style, UniquePtr<RenderStyle> &&firstLineStyle) noexcept
      : Box(krys::move(attributes), krys::move(style), krys::move(firstLineStyle), ElementBoxFlag),
        _replacedData(CreateUnique<ReplacedData>())
  {
    _replacedData->IntrinsicSize = replacedAttributes.IntrinsicSize;
    _replacedData->IntrinsicRatio = replacedAttributes.IntrinsicRatio;
  }

  ElementBox::~ElementBox() noexcept
  {
    DestroyChildren();
  }

  const Box *ElementBox::FirstInFlowChild() const noexcept
  {
    if (auto *firstChild = FirstChild())
    {
      if (firstChild->IsInFlow())
      {
        return firstChild;
      }
      return firstChild->NextInFlowSibling();
    }
    return nullptr;
  }

  const Box *ElementBox::FirstInFlowOrFloatingChild() const noexcept
  {
    if (auto *firstChild = FirstChild())
    {
      if (firstChild->IsInFlow() || firstChild->IsFloatingPositioned())
      {
        return firstChild;
      }
      return firstChild->NextInFlowOrFloatingSibling();
    }
    return nullptr;
  }

  const Box *ElementBox::FirstOutOfFlowChild() const noexcept
  {
    if (auto *firstChild = FirstChild())
    {
      if (firstChild->IsOutOfFlowPositioned())
      {
        return firstChild;
      }
      return firstChild->NextOutOfFlowSibling();
    }
    return nullptr;
  }

  const Box *ElementBox::LastInFlowChild() const noexcept
  {
    if (auto *lastChild = LastChild())
    {
      if (lastChild->IsInFlow())
      {
        return lastChild;
      }
      return lastChild->PreviousInFlowSibling();
    }
    return nullptr;
  }

  const Box *ElementBox::LastInFlowOrFloatingChild() const noexcept
  {
    if (auto *lastChild = LastChild())
    {
      if (lastChild->IsInFlow() || lastChild->IsFloatingPositioned())
      {
        return lastChild;
      }
      return lastChild->PreviousInFlowOrFloatingSibling();
    }
    return nullptr;
  }

  const Box *ElementBox::LastOutOfFlowChild() const noexcept
  {
    if (auto *lastChild = LastChild())
    {
      if (lastChild->IsOutOfFlowPositioned())
      {
        return lastChild;
      }
      return lastChild->PreviousOutOfFlowSibling();
    }
    return nullptr;
  }

  bool ElementBox::HasOutOfFlowChild() const noexcept
  {
    return FirstOutOfFlowChild() != nullptr;
  }

  void ElementBox::AppendChild(UniquePtr<Box> childRef) noexcept
  {
    InsertChild(krys::move(childRef), _lastChild.get());
  }

  void ElementBox::InsertChild(UniquePtr<Box> childRef, Box *beforeChild) noexcept
  {
    auto childBox = krys::move(childRef);
    krys_debug_assert(childBox);
    krys_debug_assert(!childBox->_parent);
    krys_debug_assert(!childBox->_previousSibling);
    krys_debug_assert(!childBox->_nextSibling);

    childBox->_parent = ShareCheckedPtr(this);

    if (!_firstChild || (beforeChild && !beforeChild->_nextSibling))
    {
      // Append as first and/or last.
      childBox->_previousSibling = _lastChild;
      auto &nextOrFirst = _lastChild ? _lastChild->_nextSibling : _firstChild;
      krys_debug_assert(!nextOrFirst);

      _lastChild = ShareCheckedPtr(childBox.get());
      nextOrFirst = krys::move(childBox);
      return;
    }

    if (!beforeChild)
    {
      // Insert as first.
      krys_debug_assert(_firstChild && _lastChild);
      _firstChild->_previousSibling = ShareCheckedPtr(childBox.get());
      childBox->_nextSibling = krys::move(_firstChild);
      _firstChild = krys::move(childBox);
      return;
    }

    krys_debug_assert(&beforeChild->Parent() == this);
    auto *nextSibling = beforeChild->_nextSibling.get();
    krys_debug_assert(nextSibling);
    childBox->_previousSibling = ShareCheckedPtr(beforeChild);
    childBox->_nextSibling = krys::move(beforeChild->_nextSibling);
    nextSibling->_previousSibling = ShareCheckedPtr(childBox.get());
    beforeChild->_nextSibling = krys::move(childBox);
  }

  void ElementBox::DestroyChildren() noexcept
  {
    _lastChild = nullptr;

    auto childToDestroy = std::exchange(_firstChild, nullptr);
    while (childToDestroy)
    {
      childToDestroy->_parent = nullptr;
      childToDestroy->_previousSibling = nullptr;
      if (childToDestroy->_nextSibling)
      {
        childToDestroy->_nextSibling->_previousSibling = nullptr;
      }
      childToDestroy = std::exchange(childToDestroy->_nextSibling, nullptr);
    }
  }

  bool ElementBox::HasIntrinsicWidth() const noexcept
  {
    return (_replacedData && _replacedData->IntrinsicSize) || Style().LogicalWidth().IsIntrinsic();
  }

  bool ElementBox::HasIntrinsicHeight() const noexcept
  {
    return (_replacedData && _replacedData->IntrinsicSize) || Style().LogicalHeight().IsIntrinsic();
  }

  bool ElementBox::HasIntrinsicRatio() const noexcept
  {
    if (!HasAspectRatio())
    {
      return false;
    }
    return _replacedData && (_replacedData->IntrinsicSize || _replacedData->IntrinsicRatio);
  }

  LayoutUnit ElementBox::IntrinsicWidth() const noexcept
  {
    krys_debug_assert(HasIntrinsicWidth());
    if (_replacedData && _replacedData->IntrinsicSize)
    {
      return _replacedData->IntrinsicSize->Width();
    }
    // FIXME: Document what invariant holds to allow not checking if the logicalWidth() is fixed.
    return LayoutUnit(Style().LogicalWidth().TryFixed()->ResolveZoom(Style().UsedZoomForLength()));
  }

  LayoutUnit ElementBox::IntrinsicHeight() const noexcept
  {
    krys_debug_assert(HasIntrinsicHeight());
    if (_replacedData && _replacedData->IntrinsicSize)
    {
      return _replacedData->IntrinsicSize->Height();
    }
    // FIXME: Document what invariant holds to allow not checking if the logicalHeight() is fixed.
    return LayoutUnit(Style().LogicalHeight().TryFixed()->ResolveZoom(Style().UsedZoomForLength()));
  }

  LayoutUnit ElementBox::IntrinsicRatio() const noexcept
  {
    krys_debug_assert(HasIntrinsicRatio() || (HasIntrinsicWidth() && HasIntrinsicHeight()));
    if (_replacedData)
    {
      if (_replacedData->IntrinsicRatio)
      {
        return *_replacedData->IntrinsicRatio;
      }
      if (_replacedData->IntrinsicSize->Height())
      {
        return _replacedData->IntrinsicSize->Width() / _replacedData->IntrinsicSize->Height();
      }
    }
    return 1;
  }

  bool ElementBox::HasAspectRatio() const noexcept
  {
    return IsImage();
  }

  RenderElement *ElementBox::RendererForIntegration() const noexcept
  {
    return Downcast<RenderElement>(Box::RendererForIntegration());
  }
}
