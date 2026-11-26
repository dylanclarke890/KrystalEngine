#pragma once

#include "Krystal.Booey/Css/Types/CSSOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Memory/RefCounted.hpp"
#include "Krystal.Net/Url.hpp"

namespace krys::boo
{
  class CSSImportRule;
  class MediaList;
  class Node;

  /// @brief A style sheet, as scripts see it. CSS style sheets are the only kind (`CSSStyleSheet`).
  /// @see https://drafts.csswg.org/cssom/#the-stylesheet-interface
  class StyleSheet : public RefCounted<StyleSheet>
  {
  public:
    virtual ~StyleSheet() noexcept = default;

#pragma region StyleSheet - https://drafts.csswg.org/cssom/#the-stylesheet-interface

    /// @brief The sheet's type: `text/css`.
    /// @see https://drafts.csswg.org/cssom/#dom-stylesheet-type
    KRYS_NODISCARD virtual CSSOMString Type() const noexcept = 0;

    /// @brief The sheet's location, or empty for a sheet that's embedded or was constructed.
    /// @see https://drafts.csswg.org/cssom/#dom-stylesheet-href
    KRYS_NODISCARD virtual CSSOMString Href() const noexcept = 0;

    /// @brief The node that put the sheet in its document, such as a `<style>` or `<link>` element.
    /// @see https://drafts.csswg.org/cssom/#dom-stylesheet-ownernode
    KRYS_NODISCARD virtual Node *OwnerNode() const noexcept = 0;

    /// @brief The sheet that imports this one with an `@import` rule.
    /// @see https://drafts.csswg.org/cssom/#dom-stylesheet-parentstylesheet
    KRYS_NODISCARD virtual StyleSheet *ParentStyleSheet() const noexcept
    {
      return nullptr;
    }

    /// @brief The sheet's title, which puts it in a named set of alternative sheets; empty if it has none.
    /// @see https://drafts.csswg.org/cssom/#dom-stylesheet-title
    KRYS_NODISCARD virtual CSSOMString Title() const noexcept = 0;

    /// @brief The media queries the sheet applies to.
    /// @see https://drafts.csswg.org/cssom/#dom-stylesheet-media
    KRYS_NODISCARD virtual MediaList *Media() const noexcept
    {
      return nullptr;
    }

    /// @brief Whether the sheet is turned off, so its rules don't apply.
    /// @see https://drafts.csswg.org/cssom/#dom-stylesheet-disabled
    KRYS_NODISCARD virtual bool Disabled() const noexcept = 0;

    /// @see https://drafts.csswg.org/cssom/#dom-stylesheet-disabled
    virtual void SetDisabled(bool disabled) noexcept = 0;

#pragma endregion

    /// @brief The `@import` rule that loaded the sheet.
    /// @see https://drafts.csswg.org/cssom/#concept-css-style-sheet-owner-css-rule
    KRYS_NODISCARD virtual CSSImportRule *OwnerRule() const noexcept
    {
      return nullptr;
    }

    /// @brief Forgets the owner node, once it no longer holds the sheet.
    virtual void ClearOwnerNode() noexcept = 0;

    /// @brief The URL the sheet's relative URLs resolve against.
    KRYS_NODISCARD virtual net::Url BaseUrl() const noexcept = 0;

    /// @brief Whether the sheet, or a sheet it imports, is still loading.
    KRYS_NODISCARD virtual bool IsLoading() const noexcept = 0;

    KRYS_NODISCARD virtual bool IsCSSStyleSheet() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool IsXSLStyleSheet() const noexcept
    {
      return false;
    }
  };
}
