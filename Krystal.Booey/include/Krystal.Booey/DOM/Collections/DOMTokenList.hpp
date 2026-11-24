/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
 * Copyright (C) 2015-2025 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Core/Base.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Containers/SmallList.hpp"
#include "Krystal.Core/Memory/CheckedPtr.hpp"
#include "Krystal.Core/Memory/UniquePtr.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  class Document;
  class Element;
}

namespace krys::boo::dom
{
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
    friend class ElementAlgorithms;
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
                 IsSupportedTokenFunction &&isSupportedToken = {}) noexcept;

  public:
    using iterator = TokenList::iterator;
    using const_iterator = TokenList::const_iterator;

    /// @brief A list bound to @p element's @p attributeName, optionally with a function that filters new
    /// values to the supported ones.
    KRYS_NODISCARD static UniquePtr<DOMTokenList>
      Create(Element &element, DOMStringAtom attributeName,
             IsSupportedTokenFunction &&isSupportedToken = {}) noexcept;

    /// @brief Marks the tokens to be parsed again from the attribute, which changed; the owner of the list
    /// calls this from its attribute change steps.
    /// @see https://dom.spec.whatwg.org/#interface-domtokenlist (its attribute change steps)
    void AssociatedAttributeValueChanged() noexcept
    {
      // 1. If localName is set’s attribute name, namespace is null, and value is null, then empty token set.
      // 2. Otherwise, if localName is set’s attribute name and namespace is null, then set set’s token set to
      //    value, parsed.
      // The token set is parsed from the attribute when it's next read (Tokens()), unless the list is setting
      // the attribute to its own token set. A null value parses as the empty string, to the empty set.
      if (_inUpdateAssociatedAttributeFromTokens)
      {
        return;
      }
      _tokensNeedUpdating = true;
    }

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
    KRYS_NODISCARD bool Contains(const DOMString &token) const noexcept
    {
      return std::ranges::contains(Tokens(), token);
    }

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
    KRYS_NODISCARD iterator begin() noexcept
    {
      return Tokens().begin();
    }

    /// @see https://dom.spec.whatwg.org/#interface-domtokenlist (its iterable<DOMString>)
    KRYS_NODISCARD iterator end() noexcept
    {
      return Tokens().end();
    }

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

    /// @see https://webidl.spec.whatwg.org/#dfn-supported-property-indices
    KRYS_NODISCARD bool IsSupportedPropertyIndex(size_t index) const noexcept
    {
      return index < Length();
    }

    /// @see https://dom.spec.whatwg.org/#concept-domtokenlist-validation
    KRYS_NODISCARD ExceptionOr<bool> ValidationSteps(DOMStringView token) const noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-dtl-update
    void UpdateSteps() noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-dtl-serialize
    KRYS_NODISCARD DOMString SerialiseSteps() const noexcept;

    KRYS_NODISCARD static ExceptionOr<void> ValidateToken(DOMStringView token) noexcept
    {
      if (token.empty())
      {
        return Exception {ExceptionCode::SyntaxError};
      }

      if (std::ranges::any_of(token, &infra::StringAlgorithms::IsASCIIWhitespace))
      {
        return Exception {ExceptionCode::InvalidCharacterError};
      }

      return {};
    }
  };
}
