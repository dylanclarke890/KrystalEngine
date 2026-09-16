#pragma once

#include "Krystal.Booey/CSS/StyleSheet.hpp"
#include "Krystal.Booey/CSS/Types/MediaListOrDOMString.hpp"
#include "Krystal.Booey/DOM/Node.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/CanMakeCheckedPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::css
{
  class MediaList;
  class CSSImportRule;
  class CSSRuleList;
  class StyleSheetContents;

  struct CSSStyleSheetInit
  {
    Maybe<dom::DOMString> BaseURL = null;
    MediaListOrDOMString Media = u8"";
    bool Disabled = false;
  };

  /// @see https://drafts.csswg.org/cssom/#the-cssstylesheet-interface
  class CSSStyleSheet : public StyleSheet, public CanMakeCheckedPtr<CSSStyleSheet>
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(CSSStyleSheet);

  private:
    /// @see https://drafts.csswg.org/cssom/#concept-css-style-sheet-location
    Maybe<dom::USVString> _location;
    /// @see https://drafts.csswg.org/cssom/#concept-css-style-sheet-parent-css-style-sheet
    RefPtr<CSSStyleSheet> _parentStyleSheet;
    /// @see https://drafts.csswg.org/cssom/#concept-css-style-sheet-owner-node
    WeakPtr<dom::Node> _ownerNode;
    /// @see https://drafts.csswg.org/cssom/#concept-css-style-sheet-owner-rule
    WeakPtr<CSSImportRule> _ownerRule;
    /// @see https://drafts.csswg.org/cssom/#concept-css-style-sheet-title
    dom::DOMString _title;
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
    WeakPtr<dom::Document> m_constructorDocument;
    /// @see https://drafts.csswg.org/cssom/#concept-css-style-sheet-location
    Maybe<dom::USVString> _stylesheetBaseUrl;

    RefPtr<StyleSheetContents> _contents;

  public:
    CSSStyleSheet(CSSStyleSheetInit options = {}) noexcept;

#pragma region CSSStyleSheet - https://drafts.csswg.org/cssom/#cssstylesheet

    /// @see https://drafts.csswg.org/cssom/#dom-cssstylesheet-ownerrule
    KRYS_NODISCARD RefPtr<CSSImportRule> OwnerRule() const noexcept;

    /// @see https://drafts.csswg.org/cssom/#dom-cssstylesheet-cssrules
    KRYS_NODISCARD dom::ExceptionOr<Ref<CSSRuleList>> CssRules() const noexcept;

    /// @see https://drafts.csswg.org/cssom/#dom-cssstylesheet-insertrule
    dom::ExceptionOr<size_t> InsertRule(CSSOMString &&rule, size_t index = 0uz) noexcept;

    /// @see https://drafts.csswg.org/cssom/#dom-cssstylesheet-deleterule
    dom::ExceptionOr<void> DeleteRule(size_t index) noexcept;

    // TODO(CSSStyleSheet): Implement Replace()
    /// @see https://drafts.csswg.org/cssom/#dom-cssstylesheet-replace
    // Promise<CSSStyleSheet> Replace(USVString text) noexcept;

    // @see https://drafts.csswg.org/cssom/#dom-cssstylesheet-replacesync
    dom::ExceptionOr<void> ReplaceSync(dom::USVString text) noexcept;

#pragma endregion

#pragma region StyleSheet - https://drafts.csswg.org/cssom/#stylesheet

    /// @see https://drafts.csswg.org/cssom/#dom-stylesheet-type
    KRYS_NODISCARD CSSOMString Type() const noexcept final
    {
      return u8"text/css";
    }

    /// @see https://drafts.csswg.org/cssom/#dom-stylesheet-href
    KRYS_NODISCARD Maybe<dom::USVString> Href() const noexcept final
    {
      return _location;
    }

    /// @see https://drafts.csswg.org/cssom/#dom-stylesheet-ownernode
    KRYS_NODISCARD RefPtr<dom::Node> OwnerNode() const noexcept final
    {
      return _ownerNode.lock();
    }

    /// @see https://drafts.csswg.org/cssom/#dom-stylesheet-parentstylesheet
    KRYS_NODISCARD RefPtr<StyleSheet> ParentStyleSheet() const noexcept final
    {
      return _parentStyleSheet;
    }

    /// @see https://drafts.csswg.org/cssom/#dom-stylesheet-title
    KRYS_NODISCARD Maybe<dom::DOMString> Title() const noexcept final
    {
      if (_title.empty())
      {
        return null;
      }

      return _title;
    }

    /// @see https://drafts.csswg.org/cssom/#dom-stylesheet-media
    KRYS_NODISCARD RefPtr<MediaList> Media() const noexcept final
    {
      krys_not_implemented();
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
