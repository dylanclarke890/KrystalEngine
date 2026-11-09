/*
 * Copyright (C) 2018-2024 Apple Inc. All rights reserved.
 * Copyright (C) 2024 Igalia S.L.
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

#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilderSVG.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGContainer.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGRoot.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/SVGResourcesCache.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGInline.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGText.hpp"
#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilderBlock.hpp"
#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilderInline.hpp"
#include "Krystal.Core/Types/SmallList.hpp"

namespace krys::boo
{
  RenderTreeBuilder::SVG::SVG(RenderTreeBuilder &builder) noexcept : _builder(builder)
  {
  }

  void RenderTreeBuilder::SVG::Attach(LegacyRenderSVGRoot &parent, RenderPtr<RenderObject> child,
                                      RenderObject *beforeChild) noexcept
  {
    auto &childToAdd = *child;
    _builder.AttachToRenderElement(parent, krys::move(child), beforeChild);
    SVGResourcesCache::ClientWasAddedToTree(childToAdd);
  }

  void RenderTreeBuilder::SVG::Attach(LegacyRenderSVGContainer &parent, RenderPtr<RenderObject> child,
                                      RenderObject *beforeChild) noexcept
  {
    auto &childToAdd = *child;
    _builder.AttachToRenderElement(parent, krys::move(child), beforeChild);
    SVGResourcesCache::ClientWasAddedToTree(childToAdd);
  }

  void RenderTreeBuilder::SVG::Attach(RenderSVGInline &parent, RenderPtr<RenderObject> child,
                                      RenderObject *beforeChild) noexcept
  {
    auto &childToAdd = *child;
    _builder.InlineBuilder().Attach(parent, krys::move(child), beforeChild);
    SVGResourcesCache::ClientWasAddedToTree(childToAdd);
    if (auto *textAncestor = RenderSVGText::LocateRenderSVGTextAncestor(parent))
    {
      textAncestor->SubtreeChildWasAdded(&childToAdd);
    }
  }

  void RenderTreeBuilder::SVG::Attach(RenderSVGText &parent, RenderPtr<RenderObject> child,
                                      RenderObject *beforeChild) noexcept
  {
    auto &childToAdd = *child;
    _builder.BlockFlowBuilder().Attach(parent, krys::move(child), beforeChild);
    SVGResourcesCache::ClientWasAddedToTree(childToAdd);
    parent.SubtreeChildWasAdded(&childToAdd);
  }

  RenderPtr<RenderObject> RenderTreeBuilder::SVG::Detach(LegacyRenderSVGRoot &parent, RenderObject &child,
                                                         WillBeDestroyed willBeDestroyed) noexcept
  {
    SVGResourcesCache::ClientWillBeRemovedFromTree(child);
    return _builder.DetachFromRenderElement(parent, child, willBeDestroyed);
  }

  RenderPtr<RenderObject> RenderTreeBuilder::SVG::Detach(RenderSVGText &parent, RenderObject &child,
                                                         WillBeDestroyed willBeDestroyed) noexcept
  {
    SVGResourcesCache::ClientWillBeRemovedFromTree(child);

    SmallList<SVGTextLayoutAttributes *, 2> affectedAttributes;
    parent.SubtreeChildWillBeRemoved(&child, affectedAttributes);
    auto takenChild =
      _builder.BlockBuilder().Detach(parent, child, willBeDestroyed, CanCollapseAnonymousBlock::Yes);
    parent.SubtreeChildWasRemoved(affectedAttributes);
    return takenChild;
  }

  RenderPtr<RenderObject> RenderTreeBuilder::SVG::Detach(RenderSVGInline &parent, RenderObject &child,
                                                         WillBeDestroyed willBeDestroyed) noexcept
  {
    SVGResourcesCache::ClientWillBeRemovedFromTree(child);

    auto *textAncestor = RenderSVGText::LocateRenderSVGTextAncestor(parent);
    if (!textAncestor)
    {
      return _builder.DetachFromRenderElement(parent, child, willBeDestroyed);
    }

    SmallList<SVGTextLayoutAttributes *, 2> affectedAttributes;
    textAncestor->SubtreeChildWillBeRemoved(&child, affectedAttributes);
    auto takenChild = _builder.DetachFromRenderElement(parent, child, willBeDestroyed);
    textAncestor->SubtreeChildWasRemoved(affectedAttributes);
    return takenChild;
  }

  RenderPtr<RenderObject> RenderTreeBuilder::SVG::Detach(LegacyRenderSVGContainer &parent,
                                                         RenderObject &child,
                                                         WillBeDestroyed willBeDestroyed) noexcept
  {
    SVGResourcesCache::ClientWillBeRemovedFromTree(child);
    return _builder.DetachFromRenderElement(parent, child, willBeDestroyed);
  }
}
