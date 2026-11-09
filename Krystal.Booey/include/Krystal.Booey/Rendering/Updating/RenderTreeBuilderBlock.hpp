/*
 * Copyright (C) 2017 Apple Inc. All rights reserved.
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

#pragma once

#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilder.hpp"

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo
{
  class RenderBlockFlow;

  /// @brief Attaches children to blocks, wrapping inline children of blocks with block children in anonymous
  /// blocks, and detaches them, merging and dropping the anonymous blocks left behind.
  class RenderTreeBuilder::Block
  {
    KRYS_NON_COPY_MOVABLE(Block);

  private:
    RenderTreeBuilder &_builder;

  public:
    explicit Block(RenderTreeBuilder &builder) noexcept;

    void Attach(RenderBlock &parent, RenderPtr<RenderObject> child, RenderObject *beforeChild) noexcept;
    void AttachIgnoringContinuation(RenderBlock &parent, RenderPtr<RenderObject> child,
                                    RenderObject *beforeChild) noexcept;

    KRYS_NODISCARD RenderPtr<RenderObject>
      Detach(RenderBlock &parent, RenderObject &child, WillBeDestroyed willBeDestroyed,
             CanCollapseAnonymousBlock canCollapseAnonymousBlock) noexcept;
    KRYS_NODISCARD RenderPtr<RenderObject>
      Detach(RenderBlockFlow &parent, RenderObject &child, WillBeDestroyed willBeDestroyed,
             CanCollapseAnonymousBlock canCollapseAnonymousBlock) noexcept;

    void DropAnonymousBoxChild(RenderBlock &parent, RenderBlock &child) noexcept;
    void ChildBecameNonInline(RenderBlock &parent, RenderElement &child) noexcept;

    KRYS_NODISCARD static RenderPtr<RenderBlockFlow>
      CreateAnonymousBlockWithStyle(dom::Document &document, const RenderStyle &style) noexcept;

  private:
    static void MoveAllChildrenToInternal(RenderBoxModelObject &from, RenderElement &newParent) noexcept;

    void RemoveLeftoverAnonymousBlock(RenderBlock &anonymousBlock) noexcept;
  };
}
