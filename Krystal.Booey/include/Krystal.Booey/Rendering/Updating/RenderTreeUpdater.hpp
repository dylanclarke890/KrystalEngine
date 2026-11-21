/*
 * Copyright (C) 2016-2025 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Rendering/Updating/RenderTreePosition.hpp"
#include "Krystal.Booey/Style/Difference.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Memory/UniquePtr.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  class ContainerNode;
  class Document;
  class Element;
  class Node;
  class Text;
}

namespace krys::boo::style
{
  struct ElementUpdate;
  class PostResolutionCallbackDisabler;
  struct TextUpdate;
  class Update;
}

namespace krys::boo
{
  class RenderElement;
  class RenderObject;
  class RenderStyle;
  class RenderTreeBuilder;
  class RenderView;

  /// @brief Applies a style update to the render tree.
  class RenderTreeUpdater
  {
    KRYS_NON_COPY_MOVABLE(RenderTreeUpdater);

    class GeneratedContent;

    struct Parent
    {
      boo::Element *Element {nullptr};
      const style::ElementUpdate *Update {nullptr};
      Maybe<RenderTreePosition> Position;

      bool DidCreateOrDestroyChildRenderer {false};
      RenderObject *PreviousChildRenderer {nullptr};
      bool HasPrecedingInFlowChild {false};

      explicit Parent(ContainerNode &root) noexcept;

      Parent(boo::Element &element, const style::ElementUpdate *update) noexcept;
    };

    enum class TeardownType : uint8
    {
      Full,
      FullAfterSlotOrShadowRootChange,
      RendererUpdate,
      RendererUpdateCancelingAnimations,
    };

  private:
    Ref<Document> _document;
    UniquePtr<style::Update> _styleUpdate;
    List<Parent> _parentStack;
    UniquePtr<GeneratedContent> _generatedContent;
    // Null for a document without a render view.
    UniquePtr<RenderTreeBuilder> _builder;

  public:
    /// @brief Adds the renderers of the `content` of @p style to @p pseudoRenderer, which has no
    /// pseudo-element node.
    static void CreateContentRenderers(RenderTreeBuilder &builder, RenderElement &pseudoRenderer,
                                       const RenderStyle &style) noexcept;

    RenderTreeUpdater(Document &document, style::PostResolutionCallbackDisabler &) noexcept;

    ~RenderTreeUpdater() noexcept;

    void Commit(UniquePtr<style::Update> styleUpdate) noexcept;

    static void TearDownRenderers(boo::Element &root) noexcept;

    static void TearDownRenderersForShadowRootInsertion(boo::Element &host) noexcept;

    static void TearDownRenderersAfterSlotChange(boo::Element &host) noexcept;

    static void TearDownRenderer(Text &text) noexcept;

  private:
    void UpdateRebuildRoots() noexcept;

    void UpdateRenderTree(ContainerNode &root) noexcept;

    static void UpdateSVGRenderer(boo::Element &element) noexcept;

    void UpdateElementRenderer(boo::Element &element, const style::ElementUpdate &elementUpdate) noexcept;

    void UpdateRendererStyle(RenderElement &renderer, RenderStyle &&newStyle,
                             style::DifferenceResult minimalStyleDifference) noexcept;

    void CreateRenderer(boo::Element &element, RenderStyle &&style) noexcept;

    void UpdateTextRenderer(Text &text, const style::TextUpdate *textUpdate,
                            const ContainerNode *root) noexcept;

    void CreateTextRenderer(Text &textNode, const style::TextUpdate *textUpdate) noexcept;

    KRYS_NODISCARD bool TextRendererIsNeeded(const Text &textNode) noexcept;

    void StorePreviousRenderer(Node &node) noexcept;

    void UpdateRenderViewStyle() noexcept;

    KRYS_NODISCARD Parent &GetParent() noexcept
    {
      return _parentStack.back();
    }

    KRYS_NODISCARD Parent &RenderingParent() noexcept;

    KRYS_NODISCARD RenderTreePosition &GetRenderTreePosition() noexcept;

    KRYS_NODISCARD GeneratedContent &GetGeneratedContent() noexcept
    {
      return *_generatedContent;
    }

    void UpdateBeforeDescendants(boo::Element &element, const style::ElementUpdate *update) noexcept;

    void UpdateAfterDescendants(boo::Element &element, const style::ElementUpdate *update) noexcept;

    void PushParent(boo::Element &element, const style::ElementUpdate *update) noexcept;

    void PopParent() noexcept;

    void PopParentsToDepth(uint32 depth) noexcept;

    KRYS_NODISCARD RenderView &GetRenderView() noexcept;

    static void TearDownRenderers(boo::Element &root, TeardownType teardownType) noexcept;

    static void TearDownRenderers(boo::Element &root, TeardownType teardownType,
                                  RenderTreeBuilder &builder) noexcept;

    static void TearDownLeftoverChildrenOfComposedTree(boo::Element &element,
                                                       RenderTreeBuilder &builder) noexcept;

    enum class NeedsRepaintAndLayout : bool
    {
      No,
      Yes,
    };

    static void
      TearDownTextRenderer(Text &text, const ContainerNode *root, RenderTreeBuilder &builder,
                           NeedsRepaintAndLayout needsRepaintAndLayout = NeedsRepaintAndLayout::Yes) noexcept;
  };
}
