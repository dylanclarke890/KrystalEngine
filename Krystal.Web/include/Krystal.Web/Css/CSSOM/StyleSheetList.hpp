#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Memory/RefCounted.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Memory/WeakPtr.hpp"

namespace krys::web::style
{
  class Scope;
}

namespace krys::web
{
  class ContainerNode;
  class Document;
  class Node;
  class ShadowRoot;
  class StyleSheet;

  /// @brief The CSS style sheets of a document or shadow root, in tree order, as scripts see them: the sheets
  /// of its `<style>` and `<link>` elements, not those it adopted. The list follows its document or shadow
  /// root until that goes, then keeps the sheets it had.
  /// @see https://drafts.csswg.org/cssom/#the-stylesheetlist-interface
  class StyleSheetList final : public RefCounted<StyleSheetList>
  {
    KRYS_NON_COPY_MOVABLE(StyleSheetList);

  private:
    // The document or shadow root, until it goes.
    WeakPtr<ContainerNode> _owner;
    // The sheets once detached from the owner.
    List<RefPtr<StyleSheet>> _detachedStyleSheets;

    explicit StyleSheetList(ContainerNode &owner) noexcept;

  public:
    ~StyleSheetList() noexcept;

    KRYS_NODISCARD static Ref<StyleSheetList> Create(Document &document) noexcept;

    KRYS_NODISCARD static Ref<StyleSheetList> Create(ShadowRoot &shadowRoot) noexcept;

#pragma region StyleSheetList - https://drafts.csswg.org/cssom/#the-stylesheetlist-interface

    /// @brief The sheet at @p index, or null past the end.
    /// @see https://drafts.csswg.org/cssom/#dom-stylesheetlist-item
    KRYS_NODISCARD StyleSheet *Item(uint32 index) const noexcept;

    /// @brief The number of sheets.
    /// @see https://drafts.csswg.org/cssom/#dom-stylesheetlist-length
    KRYS_NODISCARD uint32 Length() const noexcept;

#pragma endregion

    KRYS_NODISCARD bool IsSupportedPropertyIndex(uint32 index) const noexcept
    {
      return index < Length();
    }

    /// @brief The document or shadow root whose sheets these are, or null once detached.
    KRYS_NODISCARD Node *OwnerNode() const noexcept;

    /// @brief Keeps the sheets as they are and stops following the document or shadow root, which is going.
    void Detach() noexcept;

  private:
    KRYS_NODISCARD style::Scope *OwnerStyleScope() const noexcept;

    KRYS_NODISCARD const List<RefPtr<StyleSheet>> &StyleSheets() const noexcept;
  };
}
