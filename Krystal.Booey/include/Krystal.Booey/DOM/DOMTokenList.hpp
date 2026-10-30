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

  using IsSupportedTokenFunction = Func<bool(Document &, DOMStringView)>;

  constexpr auto DefaultIsSupportedTokenFunction = [](Document &, DOMStringView) -> bool
  {
    return true;
  };

  /// @see https://dom.spec.whatwg.org/#interface-domtokenlist
  class DOMTokenList
  {
    friend class Element;
    friend class ElementAlgorithms;
    friend class HTMLCollectionAlgorithms;

    using TokenList = SmallList<DOMString, 1>;

  private:
    DOMStringAtom _attributeName;
    mutable TokenList _tokens;
    CheckedRef<Element> _element;
    IsSupportedTokenFunction _isSupportedToken;
    /// @brief Whether the attribute changed since the tokens were last parsed from it.
    mutable bool _tokensNeedUpdating {false};
    /// @brief Whether the list is setting its own attribute, whose tokens it already has.
    bool _inUpdateAssociatedAttributeFromTokens {false};

  protected:
    DOMTokenList(Element &element, DOMStringAtom attributeName,
                 IsSupportedTokenFunction &&isSupportedToken = DefaultIsSupportedTokenFunction) noexcept;

  public:
    using iterator = TokenList::iterator;
    using const_iterator = TokenList::const_iterator;

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

#pragma region DOMTokenList - https://dom.spec.whatwg.org/#domtokenlist

    /// @see https://dom.spec.whatwg.org/#dom-domtokenlist-length
    KRYS_NODISCARD size_t Length() const noexcept
    {
      return Tokens().size();
    }

    /// @see https://dom.spec.whatwg.org/#dom-domtokenlist-item
    KRYS_NODISCARD Maybe<DOMString> Item(size_t index) const noexcept
    {
      if (index >= Tokens().size())
      {
        return null;
      }

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
      for (auto &token : tokens)
      {
        if (auto result = ValidateToken(token); result.HasException())
        {
          return result.ReleaseException();
        }
      }

      for (auto &token : tokens)
      {
        if (!std::ranges::contains(Tokens(), token))
        {
          Tokens().emplace_back(token);
        }
      }

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
      for (auto &token : tokens)
      {
        if (auto result = ValidateToken(token); result.HasException())
        {
          return result.ReleaseException();
        }
      }

      auto it = std::ranges::remove_if(Tokens(), [&](const DOMString &current)
                                       { return std::ranges::contains(tokens, current); });
      Tokens().erase(it.begin(), it.end());

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
      if (auto result = ValidateToken(token); result.HasException())
      {
        return result.ReleaseException();
      }

      if (std::ranges::contains(Tokens(), token))
      {
        if (!force.value_or(false))
        {
          auto it =
            std::ranges::remove_if(Tokens(), [&](const DOMString &current) { return current == token; });
          Tokens().erase(it.begin(), it.end());

          UpdateSteps();
          return false;
        }
        return true;
      }

      if (force.value_or(true))
      {
        Tokens().emplace_back(token);
        UpdateSteps();
        return true;
      }

      return false;
    }

    /// @see https://dom.spec.whatwg.org/#dom-domtokenlist-replace
    ExceptionOr<bool> Replace(const DOMString &token, const DOMString &newToken) noexcept
    {
      html::CEReactionsScope ceReactions;
      if (auto result = ValidateToken(token); result.HasException())
      {
        return result.ReleaseException();
      }

      if (auto result = ValidateToken(newToken); result.HasException())
      {
        return result.ReleaseException();
      }

      if (!std::ranges::contains(Tokens(), token))
      {
        return false;
      }

      std::ranges::replace(Tokens(), token, newToken);
      UpdateSteps();

      return true;
    }

    /// @see https://dom.spec.whatwg.org/#dom-domtokenlist-supports
    KRYS_NODISCARD ExceptionOr<bool> Supports(DOMStringView token) const noexcept
    {
      return ValidationSteps(token);
    }

    /// @see https://dom.spec.whatwg.org/#dom-domtokenlist-value
    KRYS_NODISCARD DOMString Value() const noexcept
    {
      return SerialiseSteps();
    }

    /// @see https://dom.spec.whatwg.org/#dom-domtokenlist-value
    void Value(DOMString &&value) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-domtokenlist-iterable
    KRYS_NODISCARD iterator begin() noexcept
    {
      return Tokens().begin();
    }

    /// @see https://dom.spec.whatwg.org/#dom-domtokenlist-iterable
    KRYS_NODISCARD iterator end() noexcept
    {
      return Tokens().end();
    }

    /// @see https://dom.spec.whatwg.org/#dom-domtokenlist-iterable
    KRYS_NODISCARD const_iterator begin() const noexcept
    {
      return Tokens().begin();
    }

    /// @see https://dom.spec.whatwg.org/#dom-domtokenlist-iterable
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