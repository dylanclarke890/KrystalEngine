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

#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/MultiColumn/RenderMultiColumnFlow.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilderBlock.hpp"
#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilderMultiColumn.hpp"

namespace krys::boo
{
  RenderTreeBuilder::BlockFlow::BlockFlow(RenderTreeBuilder &builder) noexcept : _builder(builder)
  {
  }

  void RenderTreeBuilder::BlockFlow::Attach(RenderBlockFlow &parent, RenderPtr<RenderObject> child,
                                            RenderObject *beforeChild) noexcept
  {
    if (auto *multicolumnFlow = parent.MultiColumnFlow())
    {
      auto legendAvoidsMulticolumn = parent.IsFieldset() && child->IsLegend();
      if (legendAvoidsMulticolumn)
      {
        _builder.BlockBuilder().Attach(parent, krys::move(child), nullptr);
        return;
      }

      auto legendBeforeChildIsIncorrect = parent.IsFieldset() && beforeChild && beforeChild->IsLegend();
      if (legendBeforeChildIsIncorrect)
      {
        _builder.BlockBuilder().Attach(*multicolumnFlow, krys::move(child), nullptr);
        return;
      }

      // When the before child is set to be the first child of the RenderBlockFlow, we need to readjust it to
      // be the first child of the multicol container.
      _builder.Attach(*multicolumnFlow, krys::move(child),
                      beforeChild == multicolumnFlow ? multicolumnFlow->FirstChild() : beforeChild);
      return;
    }

    auto *beforeChildOrPlaceholder = beforeChild;
    if (auto *containingFragmentedFlow = parent.EnclosingFragmentedFlow())
    {
      beforeChildOrPlaceholder =
        _builder.MultiColumnBuilder().ResolveMovedChild(*containingFragmentedFlow, beforeChild);
    }
    _builder.BlockBuilder().Attach(parent, krys::move(child), beforeChildOrPlaceholder);
  }

  void RenderTreeBuilder::BlockFlow::MoveAllChildrenIncludingFloats(
    RenderBlockFlow &from, RenderBlock &to, NormalizeAfterInsertion normalizeAfterInsertion) noexcept
  {
    _builder.MoveAllChildren(from, to, normalizeAfterInsertion);
    from.AddFloatsToNewParent(Downcast<RenderBlockFlow>(to));
  }
}
