#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/CheckedPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo
{
  class RenderBlock;
  class RenderElement;
  class RenderObject;
  class RenderStyle;
  class RenderText;
}

namespace krys::boo::layout
{
  class Box;
  class ElementBox;
  class InitialContainingBlock;
}

namespace krys::boo::layout_integration
{
  /// @brief Builds and updates the layout boxes for the content of a formatting context root renderer, which
  /// the layout formatting contexts lay out.
  class BoxTreeUpdater
  {
  private:
    RenderBlock &_rootRenderer;
    CheckedPtr<const dom::Document> _document;

  public:
    explicit BoxTreeUpdater(RenderBlock &rootRenderer) noexcept;
    BoxTreeUpdater(RenderBlock &rootRenderer, const dom::Document &document) noexcept;
    ~BoxTreeUpdater() noexcept;

    CheckedRef<layout::ElementBox> Build() noexcept;
    void TearDown() noexcept;

    static void UpdateStyle(const RenderObject &renderer) noexcept;
    static void UpdateContent(const RenderText &textRenderer) noexcept;

    const layout::Box &Insert(const RenderElement &parent, RenderObject &child,
                              const RenderObject *beforeChild = nullptr) noexcept;
    UniquePtr<layout::Box> Remove(const RenderElement &parent, RenderObject &child) noexcept;

  private:
    KRYS_NODISCARD const RenderBlock &RootRenderer() const noexcept
    {
      return _rootRenderer;
    }
    KRYS_NODISCARD RenderBlock &RootRenderer() noexcept
    {
      return _rootRenderer;
    }
    KRYS_NODISCARD const layout::ElementBox &RootLayoutBox() const noexcept;
    KRYS_NODISCARD layout::ElementBox &RootLayoutBox() noexcept;
    KRYS_NODISCARD layout::InitialContainingBlock &GetInitialContainingBlock() noexcept;

    KRYS_NODISCARD static UniquePtr<layout::Box> CreateLayoutBox(RenderObject &renderer) noexcept;
    static void AdjustStyleIfNeeded(const RenderElement &renderer, RenderStyle &style,
                                    RenderStyle *firstLineStyle) noexcept;

    void BuildTreeForInlineContent() noexcept;
    void BuildTreeForFlexContent() noexcept;
    void BuildTreeForGridContent() noexcept;
    void InsertChild(UniquePtr<layout::Box> childBox, RenderObject &childRenderer,
                     const RenderObject *beforeChild = nullptr) noexcept;
  };
}
