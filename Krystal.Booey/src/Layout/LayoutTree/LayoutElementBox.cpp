/*
 * Copyright (C) 2018 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

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
