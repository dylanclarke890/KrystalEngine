/*
 * Copyright (C) 2018-2024 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilderInline.hpp"
#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilderMultiColumn.hpp"

namespace krys::boo
{
  RenderTreeBuilder::Inline::Inline(RenderTreeBuilder &builder) noexcept : _builder(builder)
  {
  }

  void RenderTreeBuilder::Inline::Attach(RenderInline &parent, RenderPtr<RenderObject> child,
                                         RenderObject *beforeChild) noexcept
  {
    auto *beforeChildOrPlaceholder = beforeChild;
    if (auto *fragmentedFlow = parent.EnclosingFragmentedFlow())
    {
      beforeChildOrPlaceholder =
        _builder.MultiColumnBuilder().ResolveMovedChild(*fragmentedFlow, beforeChild);
    }
    AttachIgnoringContinuation(parent, krys::move(child), beforeChildOrPlaceholder);
  }

  void RenderTreeBuilder::Inline::AttachIgnoringContinuation(RenderInline &parent,
                                                             RenderPtr<RenderObject> child,
                                                             RenderObject *beforeChild) noexcept
  {
    // Make sure we don't append things after :after-generated content if we have it.
    if (!beforeChild && RenderElement::IsAfterContent(parent.LastChild()))
    {
      beforeChild = parent.LastChild();
    }

    // In blocks-in-inline case we allow blocks that may need splitting.
    if (beforeChild && beforeChild->Parent() != &parent)
    {
      beforeChild = _builder.SplitAnonymousBoxesAroundChild(parent, *beforeChild);
    }

    auto &childToAdd = *child;
    _builder.AttachToRenderElement(parent, krys::move(child), beforeChild);
    childToAdd.SetNeedsLayoutAndPreferredWidthsUpdate();
  }

  void RenderTreeBuilder::Inline::ChildBecameNonInline(RenderInline &, RenderElement &) noexcept
  {
    // Blocks stay in inline boxes: there's nothing to split without continuations.
  }
}
