#pragma once

#include "Krystal.Booey/Dom/Types/DOMString.hpp"
#include "Krystal.Booey/Dom/Types/ExceptionOr.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Containers/SmallList.hpp"
#include "Krystal.Core/Memory/UniquePtr.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  class Document;
  class Element;

  /// @brief Whether a token, in ASCII lowercase, is one of the supported tokens of a list's element and
  /// attribute name. Empty when they define no supported tokens.
  /// @see https://dom.spec.whatwg.org/#concept-supported-tokens
  using IsSupportedTokenFunction = Func<bool(Document &, DOMStringView)>;

  /// @brief An element's attribute value as an ordered set of tokens, which it keeps in step with the
  /// attribute.
  /// @see https://dom.spec.whatwg.org/#sets
  /// @see https://dom.spec.whatwg.org/#interface-domtokenlist
  class DOMTokenList
  {
    friend class boo::Element;
    friend class dom::ElementAlgorithms;
    friend class HTMLCollectionAlgorithms;

    using TokenList = SmallList<DOMString, 1>;

  private:
    /// @see https://dom.spec.whatwg.org/#domtokenlist-attribute-name
    DOMStringAtom _attributeName;
    /// @see https://dom.spec.whatwg.org/#concept-dtl-tokens
    mutable TokenList _tokens;
    /// @see https://dom.spec.whatwg.org/#domtokenlist-element
    CheckedRef<Element> _element;
    /// @see https://dom.spec.whatwg.org/#concept-supported-tokens
    IsSupportedTokenFunction _isSupportedToken;
    /// @brief Whether the attribute changed since the tokens were last parsed from it.
    mutable bool _tokensNeedUpdating {false};
    /// @brief Whether the list is setting its own attribute, whose tokens it already has.
    bool _inUpdateAssociatedAttributeFromTokens {false};

  protected:
    /// @see https://dom.spec.whatwg.org/#interface-domtokenlist (when a DOMTokenList object set is created)
    DOMTokenList(Element &element, DOMStringAtom attributeName,
                 IsSupportedTokenFunction isSupportedToken = {}) noexcept;

  public:
    using iterator = TokenList::iterator;
    using const_iterator = TokenList::const_iterator;

    /// @brief A list bound to @p element's @p attributeName, optionally with a function that filters new
    /// values to the supported ones.
    KRYS_NODISCARD static UniquePtr<DOMTokenList>
      Create(Element &element, DOMStringAtom attributeName,
             IsSupportedTokenFunction isSupportedToken = {}) noexcept;

#pragma region DOMTokenList - https://dom.spec.whatwg.org/#domtokenlist

    /// @see https://dom.spec.whatwg.org/#dom-domtokenlist-length
    KRYS_NODISCARD size_t Length() const noexcept
    {
      return Tokens().size();
    }

    /// @see https://dom.spec.whatwg.org/#dom-domtokenlist-item
    KRYS_NODISCARD Maybe<DOMString> Item(size_t index) const noexcept
    {
      // 1. If index is equal to or greater than this’s token set’s size, then return null.
      if (index >= Tokens().size())
      {
        return null;
      }

      // 2. Return this’s token set[index].
      return Tokens()[index];
    }

    /// @see https://dom.spec.whatwg.org/#dom-domtokenlist-item
    KRYS_NODISCARD Maybe<DOMString> operator[](size_t index) const noexcept
    {
      return Item(index);
    }

    /// @see https://dom.spec.whatwg.org/#dom-domtokenlist-contains
    KRYS_NODISCARD bool Contains(const DOMString &token) const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-domtokenlist-add
    ExceptionOr<void> Add(const List<DOMString> &tokens) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-domtokenlist-add
    ExceptionOr<void> Add(const DOMString &token) noexcept
    {
      return Add(List<DOMString> {token});
    }

    /// @see https://dom.spec.whatwg.org/#dom-domtokenlist-remove
    ExceptionOr<void> Remove(const List<DOMString> &tokens) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-domtokenlist-remove
    ExceptionOr<void> Remove(const DOMString &token) noexcept
    {
      return Remove(List<DOMString> {token});
    }

    /// @see https://dom.spec.whatwg.org/#dom-domtokenlist-toggle
    ExceptionOr<bool> Toggle(const DOMString &token, Maybe<bool> force = null) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-domtokenlist-replace
    ExceptionOr<bool> Replace(const DOMString &token, const DOMString &newToken) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-domtokenlist-supports
    KRYS_NODISCARD ExceptionOr<bool> Supports(DOMStringView token) const noexcept
    {
      // 1. Let result be the return value of validation steps called with token.
      // 2. Return result.
      return ValidationSteps(token);
    }

    /// @see https://dom.spec.whatwg.org/#dom-domtokenlist-value
    KRYS_NODISCARD DOMString Value() const noexcept
    {
      return SerialiseSteps();
    }

    /// @see https://dom.spec.whatwg.org/#dom-domtokenlist-value
    void Value(DOMString value) noexcept;

    /// @see https://dom.spec.whatwg.org/#interface-domtokenlist (its iterable<DOMString>)
    KRYS_NODISCARD const_iterator begin() const noexcept
    {
      return Tokens().begin();
    }

    /// @see https://dom.spec.whatwg.org/#interface-domtokenlist (its iterable<DOMString>)
    KRYS_NODISCARD const_iterator end() const noexcept
    {
      return Tokens().end();
    }

#pragma endregion

    /// @brief Marks the tokens to be parsed again from the attribute, which changed; the owner of the list
    /// calls this from its attribute change steps.
    /// @see https://dom.spec.whatwg.org/#interface-domtokenlist (its attribute change steps)
    void AssociatedAttributeValueChanged() noexcept
    {
      if (_inUpdateAssociatedAttributeFromTokens)
      {
        return;
      }

      _tokensNeedUpdating = true;
    }

    /// @see https://webidl.spec.whatwg.org/#dfn-supported-property-indices
    KRYS_NODISCARD bool IsSupportedPropertyIndex(size_t index) const noexcept
    {
      return index < Length();
    }

  private:
    /// @brief The token set, parsed again from the attribute if it changed.
    KRYS_NODISCARD TokenList &Tokens() const noexcept
    {
      if (_tokensNeedUpdating)
      {
        UpdateTokensFromAttributeValue();
      }

      return _tokens;
    }

    /// @brief Sets the token set to the attribute's value, parsed.
    void UpdateTokensFromAttributeValue() const noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-domtokenlist-validation
    KRYS_NODISCARD ExceptionOr<bool> ValidationSteps(DOMStringView token) const noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-dtl-update
    void UpdateSteps() noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-dtl-serialize
    KRYS_NODISCARD DOMString SerialiseSteps() const noexcept;

    KRYS_NODISCARD static ExceptionOr<void> ValidateToken(DOMStringView token) noexcept;
  };
}
