#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/List.hpp"
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
  class RenderStyle;

  /// @brief Applies a style update to the render tree.
  // FIXME: There's no render tree yet. Elements get renderers that aren't attached to one (the element owns
  // its renderer), the style of elements with `display: contents` or `display: none` is kept as upstream, and
  // the render view's style is on the document. Text renderers, generated content (::before, ::after,
  // ::marker, ::backdrop), rebuild roots, the top layer and layout come with the render tree.
  class RenderTreeUpdater
  {
    KRYS_NON_COPY_MOVABLE(RenderTreeUpdater);

    struct Parent
    {
      dom::Element *Element {nullptr};
      const style::ElementUpdate *Update {nullptr};

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
    void CreateRenderer(dom::Element &element, RenderStyle &&style) noexcept;
    void UpdateRenderViewStyle() noexcept;

    void PushParent(dom::Element &element, const style::ElementUpdate *update) noexcept;
    void PopParent() noexcept;
    void PopParentsToDepth(uint32 depth) noexcept;

    static void TearDownRenderers(dom::Element &root, TeardownType teardownType) noexcept;
    static void TearDownLeftoverChildrenOfComposedTree(dom::Element &element) noexcept;
  };
}
