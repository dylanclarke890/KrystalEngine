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
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::dom
{
  class ContainerNode;
  class Document;
  class Element;
  class Node;
  class Text;
}

namespace krys::boo::style
{
  class PostResolutionCallbackDisabler;
  class Update;
  struct ElementUpdate;
  struct TextUpdate;
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
      dom::Element *Element {nullptr};
      const style::ElementUpdate *Update {nullptr};
      Maybe<RenderTreePosition> Position;

      bool DidCreateOrDestroyChildRenderer {false};
      RenderObject *PreviousChildRenderer {nullptr};
      bool HasPrecedingInFlowChild {false};

      explicit Parent(dom::ContainerNode &root) noexcept;
      Parent(dom::Element &element, const style::ElementUpdate *update) noexcept;
    };

    enum class TeardownType : uint8
    {
      Full,
      FullAfterSlotOrShadowRootChange,
      RendererUpdate,
      RendererUpdateCancelingAnimations
    };

  private:
    Ref<dom::Document> _document;
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

    RenderTreeUpdater(dom::Document &document, style::PostResolutionCallbackDisabler &) noexcept;
    ~RenderTreeUpdater() noexcept;

    void Commit(UniquePtr<style::Update> styleUpdate) noexcept;

    static void TearDownRenderers(dom::Element &root) noexcept;
    static void TearDownRenderersForShadowRootInsertion(dom::Element &host) noexcept;
    static void TearDownRenderersAfterSlotChange(dom::Element &host) noexcept;
    static void TearDownRenderer(dom::Text &text) noexcept;

  private:
    void UpdateRebuildRoots() noexcept;
    void UpdateRenderTree(dom::ContainerNode &root) noexcept;
    void UpdateSVGRenderer(dom::Element &element) noexcept;
    void UpdateElementRenderer(dom::Element &element, const style::ElementUpdate &elementUpdate) noexcept;
    void UpdateRendererStyle(RenderElement &renderer, RenderStyle &&newStyle,
                             style::DifferenceResult minimalStyleDifference) noexcept;
    void CreateRenderer(dom::Element &element, RenderStyle &&style) noexcept;
    void UpdateTextRenderer(dom::Text &text, const style::TextUpdate *textUpdate,
                            const dom::ContainerNode *root) noexcept;
    void CreateTextRenderer(dom::Text &textNode, const style::TextUpdate *textUpdate) noexcept;
    KRYS_NODISCARD bool TextRendererIsNeeded(const dom::Text &textNode) noexcept;
    void StorePreviousRenderer(dom::Node &node) noexcept;
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

    void UpdateBeforeDescendants(dom::Element &element, const style::ElementUpdate *update) noexcept;
    void UpdateAfterDescendants(dom::Element &element, const style::ElementUpdate *update) noexcept;

    void PushParent(dom::Element &element, const style::ElementUpdate *update) noexcept;
    void PopParent() noexcept;
    void PopParentsToDepth(uint32 depth) noexcept;

    KRYS_NODISCARD RenderView &GetRenderView() noexcept;

    static void TearDownRenderers(dom::Element &root, TeardownType teardownType) noexcept;
    static void TearDownRenderers(dom::Element &root, TeardownType teardownType,
                                  RenderTreeBuilder &builder) noexcept;
    static void TearDownLeftoverChildrenOfComposedTree(dom::Element &element,
                                                       RenderTreeBuilder &builder) noexcept;
    enum class NeedsRepaintAndLayout : bool
    {
      No,
      Yes
    };
    static void
      TearDownTextRenderer(dom::Text &text, const dom::ContainerNode *root, RenderTreeBuilder &builder,
                           NeedsRepaintAndLayout needsRepaintAndLayout = NeedsRepaintAndLayout::Yes) noexcept;
  };
}
