#pragma once

#include "Krystal.Booey/Style/Change.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeCheckedPtr.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/ListHashSet.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include <limits>

namespace krys::boo
{
  class RenderStyle;
}

namespace krys::boo::dom
{
  class ContainerNode;
  class Document;
  class Element;
  class Text;
}

namespace krys::boo::svg
{
  class SVGElement;
}

namespace krys::boo::style
{
  /// @brief The new style of an element and how it changed.
  struct ElementUpdate
  {
    UniquePtr<RenderStyle> Style;
    EnumSet<Change> Changes {};
    bool RecompositeLayer {false};
    bool MayNeedRebuildRoot {false};
  };

  /// @brief The part of a text node whose rendering needs updating, and the style it inherits through a
  /// `display: contents` parent.
  struct TextUpdate
  {
    uint32 Offset {0};
    uint32 Length {std::numeric_limits<uint32>::max()};
    Maybe<UniquePtr<RenderStyle>> InheritedDisplayContentsStyle;
  };

  /// @brief The result of a style resolution: the new styles of the elements and text nodes whose rendering
  /// changed, and the roots of the subtrees they're in.
  class Update final : public CanMakeCheckedPtr<Update>
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(Update);
    KRYS_NON_COPY_MOVABLE(Update);

  private:
    Ref<dom::Document> _document;
    ListHashSet<RefPtr<dom::ContainerNode>> _roots;
    ListHashSet<RefPtr<dom::Element>> _rebuildRoots;
    HashMap<RefPtr<const dom::Element>, ElementUpdate> _elements;
    HashMap<RefPtr<const dom::Text>, TextUpdate> _texts;
    UniquePtr<RenderStyle> _initialContainingBlockUpdate;

  public:
    explicit Update(dom::Document &document) noexcept;
    ~Update() noexcept;

    KRYS_NODISCARD const ListHashSet<RefPtr<dom::ContainerNode>> &Roots() const noexcept
    {
      return _roots;
    }

    KRYS_NODISCARD ListHashSet<RefPtr<dom::Element>> TakeRebuildRoots() noexcept
    {
      return krys::move(_rebuildRoots);
    }

    KRYS_NODISCARD const ElementUpdate *GetElementUpdate(const dom::Element &element) const noexcept;
    KRYS_NODISCARD ElementUpdate *GetElementUpdate(const dom::Element &element) noexcept;

    KRYS_NODISCARD const TextUpdate *GetTextUpdate(const dom::Text &text) const noexcept;

    KRYS_NODISCARD const RenderStyle *InitialContainingBlockUpdate() const noexcept
    {
      return _initialContainingBlockUpdate.get();
    }

    /// @brief The element's new style, or else the style it's rendered with.
    KRYS_NODISCARD const RenderStyle *ElementStyle(const dom::Element &element) const noexcept;
    KRYS_NODISCARD RenderStyle *ElementStyle(const dom::Element &element) noexcept;

    KRYS_NODISCARD const dom::Document &Document() const noexcept
    {
      return *_document;
    }

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return !Size();
    }

    KRYS_NODISCARD uint32 Size() const noexcept
    {
      return static_cast<uint32>(_elements.size() + _texts.size());
    }

    void AddElement(dom::Element &element, dom::Element *parent, ElementUpdate &&elementUpdate) noexcept;
    void AddText(dom::Text &text, dom::Element *parent, TextUpdate &&textUpdate) noexcept;
    void AddText(dom::Text &text, TextUpdate &&textUpdate) noexcept;
    void AddSVGRendererUpdate(svg::SVGElement &element) noexcept;
    void AddInitialContainingBlockUpdate(UniquePtr<RenderStyle> style) noexcept;

  private:
    void AddPossibleRoot(dom::Element *element) noexcept;
    void AddPossibleRebuildRoot(dom::Element &element, dom::Element *parent) noexcept;
  };
}
