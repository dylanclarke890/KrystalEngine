#pragma once

#include "Krystal.HTML/CSS/Dicts/CSSStyleSheetInit.hpp"
#include "Krystal.HTML/CSS/StyleSheet.hpp"
#include "Krystal.HTML/DOM/Node.hpp"
#include "Krystal.Lib/Mixins/CanMakeCheckedPtr.hpp"
#include "Krystal.Lib/Pointers/WeakPtr.hpp"

namespace Krys::HTML
{
  class MediaList;
  class CSSImportRule;
  class CSSRuleList;
  class StyleSheetContents;

  /// @see https://drafts.csswg.org/cssom/#the-cssstylesheet-interface
  class CSSStyleSheet : public StyleSheet, public CanMakeCheckedPtr<CSSStyleSheet>
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(CSSStyleSheet);

  private:
    /// @see https://drafts.csswg.org/cssom/#concept-css-style-sheet-location
    Maybe<USVString> _location;
    /// @see https://drafts.csswg.org/cssom/#concept-css-style-sheet-parent-css-style-sheet
    RefPtr<CSSStyleSheet> _parentStyleSheet;
    /// @see https://drafts.csswg.org/cssom/#concept-css-style-sheet-owner-node
    WeakPtr<Node> _ownerNode;
    /// @see https://drafts.csswg.org/cssom/#concept-css-style-sheet-owner-rule
    WeakPtr<CSSImportRule> _ownerRule;
    /// @see https://drafts.csswg.org/cssom/#concept-css-style-sheet-title
    DOMString _title;
    /// @see https://drafts.csswg.org/cssom/#concept-css-style-sheet-media
    RefPtr<MediaList> _media;
    /// @see https://drafts.csswg.org/cssom/#concept-css-style-sheet-alternate-flag
    bool _alternate : 1 {false};
    /// @see https://drafts.csswg.org/cssom/#concept-css-style-sheet-disabled-flag
    bool _disabled : 1 {false};
    /// @see https://drafts.csswg.org/cssom/#concept-css-style-sheet-origin-clean-flag
    bool _originClean : 1 {false};
    /// @see https://drafts.csswg.org/cssom/#concept-css-style-sheet-origin-constructed-flag
    bool _constructed : 1 {false};
    /// @see https://drafts.csswg.org/cssom/#concept-css-style-sheet-origin-disallow-modification-flag
    bool _disallowModification : 1 {false};
    /// @see https://drafts.csswg.org/cssom/#concept-css-style-sheet-css-rules
    mutable UniquePtr<CSSRuleList> _cssRules;
    /// @see https://drafts.csswg.org/cssom/#concept-css-style-sheet-constructor-document
    WeakPtr<Document> m_constructorDocument;
    /// @see https://drafts.csswg.org/cssom/#concept-css-style-sheet-location
    Maybe<USVString> _stylesheetBaseUrl;

    RefPtr<StyleSheetContents> _contents;

  public:
    CSSStyleSheet(CSSStyleSheetInit options = {}) noexcept;

#pragma region CSSStyleSheet - https://drafts.csswg.org/cssom/#cssstylesheet

    /// @see https://drafts.csswg.org/cssom/#dom-cssstylesheet-ownerrule
    KRYS_NODISCARD RefPtr<CSSImportRule> OwnerRule() const noexcept;

    /// @see https://drafts.csswg.org/cssom/#dom-cssstylesheet-cssrules
    KRYS_NODISCARD ExceptionOr<Ref<CSSRuleList>> CssRules() const noexcept;

    /// @see https://drafts.csswg.org/cssom/#dom-cssstylesheet-insertrule
    ExceptionOr<size_t> InsertRule(CSSOMString&& rule, size_t index = 0uz) noexcept;

    /// @see https://drafts.csswg.org/cssom/#dom-cssstylesheet-deleterule
    ExceptionOr<void> DeleteRule(size_t index) noexcept;

    // TODO(CSSStyleSheet): Implement Replace()
    /// @see https://drafts.csswg.org/cssom/#dom-cssstylesheet-replace
    // Promise<CSSStyleSheet> Replace(USVString text) noexcept;

    // @see https://drafts.csswg.org/cssom/#dom-cssstylesheet-replacesync
    ExceptionOr<void> ReplaceSync(USVString text) noexcept;

#pragma endregion

#pragma region StyleSheet - https://drafts.csswg.org/cssom/#stylesheet

    /// @see https://drafts.csswg.org/cssom/#dom-stylesheet-type
    KRYS_NODISCARD CSSOMString Type() const noexcept final
    {
      return u8"text/css";
    }

    /// @see https://drafts.csswg.org/cssom/#dom-stylesheet-href
    KRYS_NODISCARD Maybe<USVString> Href() const noexcept final
    {
      return _location;
    }

    /// @see https://drafts.csswg.org/cssom/#dom-stylesheet-ownernode
    KRYS_NODISCARD RefPtr<Node> OwnerNode() const noexcept final
    {
      return _ownerNode.lock();
    }

    /// @see https://drafts.csswg.org/cssom/#dom-stylesheet-parentstylesheet
    KRYS_NODISCARD RefPtr<StyleSheet> ParentStyleSheet() const noexcept final
    {
      return _parentStyleSheet;
    }

    /// @see https://drafts.csswg.org/cssom/#dom-stylesheet-title
    KRYS_NODISCARD Maybe<DOMString> Title() const noexcept final
    {
      if (_title.empty())
      {
        return Null;
      }

      return _title;
    }

    /// @see https://drafts.csswg.org/cssom/#dom-stylesheet-media
    KRYS_NODISCARD RefPtr<MediaList> Media() const noexcept final
    {
      // TODO(CSSStyleSheet): Implement Media()
      return nullptr;
    }

    /// @see https://drafts.csswg.org/cssom/#dom-stylesheet-disabled
    KRYS_NODISCARD bool Disabled() const noexcept final
    {
      return _disabled;
    }

    /// @see https://drafts.csswg.org/cssom/#dom-stylesheet-disabled
    void Disabled(bool value) noexcept final
    {
      _disabled = value;
    }

#pragma endregion
  };
}