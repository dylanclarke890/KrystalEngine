#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::css
{
  class CSSStyleSheet;
}

namespace krys::boo::style
{
  class Scope;
}

namespace krys::boo::dom
{
  class Element;

  /// @brief The style sheet of an element whose sheet is its own text (a `style` element): creates the sheet
  /// from the element's child text and keeps it, and the element's place among its scope's style sheet
  /// candidates, up to date.
  /// @note The element's children are still being parsed while `Element::IsFinishedParsingChildren()` is
  /// false.
  /// @see https://html.spec.whatwg.org/#update-a-style-block
  class InlineStyleSheetOwner
  {
    KRYS_NON_COPY_MOVABLE(InlineStyleSheetOwner);

  private:
    bool _loading {false};
    DOMStringAtom _contentType {DOMStringAtom::Null()};
    DOMStringAtom _media {DOMStringAtom::Null()};
    RefPtr<css::CSSStyleSheet> _sheet;
    WeakPtr<style::Scope> _styleScope;

  public:
    InlineStyleSheetOwner() noexcept;
    ~InlineStyleSheetOwner() noexcept;

    void SetContentType(DOMStringAtom contentType) noexcept
    {
      _contentType = contentType;
    }

    void SetMedia(DOMStringAtom media) noexcept
    {
      _media = media;
    }

    KRYS_NODISCARD css::CSSStyleSheet *Sheet() const noexcept;

    KRYS_NODISCARD bool IsLoading() const noexcept;
    bool SheetLoaded(Element &element) noexcept;
    void StartLoadingDynamicSheet(Element &element) noexcept;

    void InsertedIntoDocument(Element &element) noexcept;
    void RemovedFromDocument(Element &element) noexcept;
    void ClearDocumentData(Element &element) noexcept;
    void ChildrenChanged(Element &element) noexcept;
    void FinishParsingChildren(Element &element) noexcept;

    KRYS_NODISCARD style::Scope *StyleScope() noexcept;

  private:
    void CreateSheet(Element &element, const DOMString &text) noexcept;
    void CreateSheetFromTextContents(Element &element) noexcept;
    void ClearSheet() noexcept;
  };
}
