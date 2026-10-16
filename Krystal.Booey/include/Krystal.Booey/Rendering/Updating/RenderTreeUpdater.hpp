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
}

namespace krys::boo::style
{
  class PostResolutionCallbackDisabler;
  class Update;
  struct ElementUpdate;
}

namespace krys::boo
{
  class RenderElement;
  class RenderStyle;
  class RenderTreeBuilder;
  class RenderView;

  /// @brief Applies a style update to the render tree.
  // FIXME: Text renderers, the renderers of generated content, ::marker, ::backdrop, the anonymous box rules,
  // rebuild roots, the top layer and layout come with the rest of the render tree.
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
    RenderTreeUpdater(dom::Document &document, style::PostResolutionCallbackDisabler &) noexcept;
    ~RenderTreeUpdater() noexcept;

    void Commit(UniquePtr<style::Update> styleUpdate) noexcept;

    static void TearDownRenderers(dom::Element &root) noexcept;
    static void TearDownRenderersForShadowRootInsertion(dom::Element &host) noexcept;
    static void TearDownRenderersAfterSlotChange(dom::Element &host) noexcept;

  private:
    void UpdateRenderTree(dom::ContainerNode &root) noexcept;
    void UpdateElementRenderer(dom::Element &element, const style::ElementUpdate &elementUpdate) noexcept;
    void UpdateRendererStyle(RenderElement &renderer, RenderStyle &&newStyle,
                             style::DifferenceResult minimalStyleDifference) noexcept;
    void CreateRenderer(dom::Element &element, RenderStyle &&style) noexcept;
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
  };
}
