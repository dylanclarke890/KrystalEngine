#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/DOM/Types/QualifiedName.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Core/Base.hpp"
#include "Krystal.Core/Types/CheckedPtr.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/SmallList.hpp"

namespace krys::boo::dom
{
  class Element;
  class Document;

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
    friend class Element;
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

    /// @brief Marks the tokens to be parsed again from the attribute, which changed; the owner of the list
    /// calls this from its attribute change steps.
    /// @see https://dom.spec.whatwg.org/#interface-domtokenlist (its attribute change steps)
    void AssociatedAttributeValueChanged() noexcept
    {
      // 1. If localName is set’s attribute name, namespace is null, and value is null, then empty token set.
      // 2. Otherwise, if localName is set’s attribute name and namespace is null, then set set’s token set to
      // value,
      //    parsed.
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
    ExceptionOr<void> Add(const List<DOMString> &tokens) noexcept
    {
      html::CEReactionsScope ceReactions;
      // 1. For each token of tokens:
      for (auto &token : tokens)
      {
        // 1a. If token is the empty string, then throw a "SyntaxError" DOMException.
        // 1b. If token contains any ASCII whitespace, then throw an "InvalidCharacterError" DOMException.
        if (auto result = ValidateToken(token); result.HasException())
        {
          return result.ReleaseException();
        }
      }

      // 2. For each token of tokens: append token to this’s token set.
      for (auto &token : tokens)
      {
        if (!std::ranges::contains(Tokens(), token))
        {
          Tokens().emplace_back(token);
        }
      }

      // 3. Run the update steps.
      UpdateSteps();

      return {};
    }

    /// @see https://dom.spec.whatwg.org/#dom-domtokenlist-add
    ExceptionOr<void> Add(const DOMString &token) noexcept
    {
      return Add(List<DOMString> {token});
    }

    /// @see https://dom.spec.whatwg.org/#dom-domtokenlist-remove
    ExceptionOr<void> Remove(const List<DOMString> &tokens) noexcept
    {
      html::CEReactionsScope ceReactions;
      // 1. For each token of tokens:
      for (auto &token : tokens)
      {
        // 1a. If token is the empty string, then throw a "SyntaxError" DOMException.
        // 1b. If token contains any ASCII whitespace, then throw an "InvalidCharacterError" DOMException.
        if (auto result = ValidateToken(token); result.HasException())
        {
          return result.ReleaseException();
        }
      }

      // 2. For each token of tokens: remove token from this’s token set.
      auto it = std::ranges::remove_if(Tokens(), [&](const DOMString &current)
                                       { return std::ranges::contains(tokens, current); });
      Tokens().erase(it.begin(), it.end());

      // 3. Run the update steps.
      UpdateSteps();

      return {};
    }

    /// @see https://dom.spec.whatwg.org/#dom-domtokenlist-remove
    ExceptionOr<void> Remove(const DOMString &token) noexcept
    {
      return Remove(List<DOMString> {token});
    }

    /// @see https://dom.spec.whatwg.org/#dom-domtokenlist-toggle
    ExceptionOr<bool> Toggle(const DOMString &token, Maybe<bool> force = null) noexcept
    {
      html::CEReactionsScope ceReactions;
      // 1. If token is the empty string, then throw a "SyntaxError" DOMException.
      // 2. If token contains any ASCII whitespace, then throw an "InvalidCharacterError" DOMException.
      if (auto result = ValidateToken(token); result.HasException())
      {
        return result.ReleaseException();
      }

      // 3. If this’s token set[token] exists:
      if (std::ranges::contains(Tokens(), token))
      {
        // 3a. If force is either not given or is false, then remove token from this’s token set, run the
        // update
        //     steps and return false.
        if (!force.value_or(false))
        {
          auto it =
            std::ranges::remove_if(Tokens(), [&](const DOMString &current) { return current == token; });
          Tokens().erase(it.begin(), it.end());

          UpdateSteps();
          return false;
        }

        // 3b. Return true.
        return true;
      }

      // 4. Otherwise, if force not given or is true, append token to this’s token set, run the update steps,
      // and
      //    return true.
      if (force.value_or(true))
      {
        Tokens().emplace_back(token);
        UpdateSteps();
        return true;
      }

      // 5. Return false.
      return false;
    }

    /// @see https://dom.spec.whatwg.org/#dom-domtokenlist-replace
    ExceptionOr<bool> Replace(const DOMString &token, const DOMString &newToken) noexcept
    {
      html::CEReactionsScope ceReactions;
      // 1. If either token or newToken is the empty string, then throw a "SyntaxError" DOMException.
      // 2. If either token or newToken contains any ASCII whitespace, then throw an "InvalidCharacterError"
      //    DOMException.
      if (token.empty() || newToken.empty())
      {
        return Exception {ExceptionCode::SyntaxError};
      }

      if (auto result = ValidateToken(token); result.HasException())
      {
        return result.ReleaseException();
      }

      if (auto result = ValidateToken(newToken); result.HasException())
      {
        return result.ReleaseException();
      }

      // 3. If this’s token set does not contain token, then return false.
      auto &tokens = Tokens();
      auto tokenIt = std::ranges::find(tokens, token);
      if (tokenIt == tokens.end())
      {
        return false;
      }

      // 4. Replace token in this’s token set with newToken.
      // The first of token and newToken becomes newToken, and the other is removed.
      auto newTokenIt = std::ranges::find(tokens, newToken);
      if (newTokenIt == tokens.end())
      {
        *tokenIt = newToken;
      }
      else if (newTokenIt > tokenIt)
      {
        *tokenIt = newToken;
        tokens.erase(newTokenIt);
      }
      else if (newTokenIt < tokenIt)
      {
        tokens.erase(tokenIt);
      }

      // 5. Run the update steps.
      UpdateSteps();

      // 6. Return true.
      return true;
    }

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

    KRYS_NODISCARD ExceptionOr<void> ValidateToken(DOMStringView token) const noexcept
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