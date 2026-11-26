#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Memory/WeakPtr.hpp"
#include "Krystal.Web/Dom/Types/DOMString.hpp"

namespace krys::web
{
  class CSSStyleSheet;
  class Element;
}

namespace krys::web::style
{
  class Scope;
}

namespace krys::web::html
{
  /// @brief The style sheet of an HTML or SVG `style` element, made from the element's child text. It
  /// [updates](https://html.spec.whatwg.org/multipage/semantics.html#update-a-style-block) the sheet as the element is connected,
  /// disconnected or has its text changed, and keeps the element among its style scope's sheet candidates.
  /// @note The element's members call these with themselves as @p element.
  class InlineStyleSheetOwner
  {
    KRYS_NON_COPY_MOVABLE(InlineStyleSheetOwner);

  private:
    /// @brief Whether the sheet is being parsed.
    bool _loading {false};
    /// @brief The element's `type` attribute.
    DOMStringAtom _contentType {null};
    /// @brief The element's `media` attribute.
    DOMStringAtom _media {null};
    RefPtr<CSSStyleSheet> _sheet;
    /// @brief The scope the element was last connected in.
    WeakPtr<style::Scope> _styleScope;

  public:
    InlineStyleSheetOwner() noexcept;

    ~InlineStyleSheetOwner() noexcept;

    /// @brief Sets the `type` the next update checks.
    void SetContentType(DOMStringAtom contentType) noexcept
    {
      _contentType = krys::move(contentType);
    }

    /// @brief Sets the `media` the next update gives the sheet.
    void SetMedia(DOMStringAtom media) noexcept
    {
      _media = krys::move(media);
    }

    /// @brief The element's sheet, or null if it has none (it's disconnected, or its type isn't CSS).
    KRYS_NODISCARD CSSStyleSheet *Sheet() const noexcept;

    /// @brief Whether the sheet is being parsed, or is waiting for its imports.
    KRYS_NODISCARD bool IsLoading() const noexcept;

    /// @brief The scope the element was last connected in, or null if it never was (or the scope is gone).
    KRYS_NODISCARD style::Scope *StyleScope() noexcept;

    /// @brief Takes the element off its scope's pending sheets once the sheet has loaded.
    /// @return Whether it had: false while the sheet or its imports are still loading.
    bool SheetLoaded(Element &element) noexcept;

    /// @brief Puts the element back on its scope's pending sheets, as an import has started loading.
    void StartLoadingDynamicSheet(Element &element) noexcept;

    /// @brief Adds the element to its scope's sheet candidates, and makes its sheet unless its children
    /// are still being parsed.
    void InsertedIntoDocument(Element &element) noexcept;

    /// @brief Takes the element out of its scope, and drops its sheet.
    void RemovedFromDocument(Element &element) noexcept;

    /// @brief Detaches the sheet from the element and takes the element out of its scope, as the element
    /// is destroyed.
    void ClearDocumentData(Element &element) noexcept;

    /// @brief Remakes the sheet from the element's new text, if it's connected and finished parsing.
    void ChildrenChanged(Element &element) noexcept;

    /// @brief Makes the sheet from the parsed text, if the element is connected.
    void FinishParsingChildren(Element &element) noexcept;

  private:
    /// @brief The update a style block algorithm, with @p text as the element's child text content.
    void UpdateSheet(Element &element, const DOMString &text) noexcept;

    void UpdateSheetFromChildText(Element &element) noexcept;

    void ClearSheet() noexcept;
  };
}
