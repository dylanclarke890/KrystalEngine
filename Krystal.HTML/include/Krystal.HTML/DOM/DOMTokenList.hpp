#pragma once

#include "Krystal.HTML/DOM/Types/DOMString.hpp"
#include "Krystal.HTML/DOM/Types/QualifiedName.hpp"
#include "Krystal.HTML/Infra/StringAlgorithms.hpp"
#include "Krystal.HTML/Types/ExceptionOr.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/CheckedPtr.hpp"
#include "Krystal.Lib/Pointers/ReferenceWrapper.hpp"
#include "Krystal.Lib/Types/Func.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Lib/Types/SmallList.hpp"

namespace Krys::HTML
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
    friend class ElementAlgorithms;
    friend class ExtensibilityHooks;
    friend class HTMLCollectionAlgorithms;

    using TokenList = SmallList<DOMString, 1>;

  private:
    DOMStringAtom _attributeName;
    TokenList _tokens;
    CheckedRef<Element> _element;
    IsSupportedTokenFunction _isSupportedToken;

  protected:
    DOMTokenList(Element &element, DOMStringAtom attributeName,
                 IsSupportedTokenFunction &&isSupportedToken = DefaultIsSupportedTokenFunction) noexcept;

  public:
    using iterator = TokenList::iterator;
    using const_iterator = TokenList::const_iterator;

#pragma region DOMTokenList - https://dom.spec.whatwg.org/#domtokenlist

    /// @see https://dom.spec.whatwg.org/#dom-domtokenlist-length
    KRYS_NODISCARD size_t Length() const noexcept
    {
      return _tokens.size();
    }

    /// @see https://dom.spec.whatwg.org/#dom-domtokenlist-item
    KRYS_NODISCARD Maybe<DOMString> Item(size_t index) const noexcept
    {
      if (index >= _tokens.size())
      {
        return Null;
      }

      return _tokens[index];
    }

    /// @see https://dom.spec.whatwg.org/#dom-domtokenlist-item
    KRYS_NODISCARD Maybe<DOMString> operator[](size_t index) const noexcept
    {
      return Item(index);
    }

    /// @see https://dom.spec.whatwg.org/#dom-domtokenlist-contains
    KRYS_NODISCARD bool Contains(const DOMString &token) const noexcept
    {
      return std::ranges::contains(_tokens, token);
    }

    /// @see https://dom.spec.whatwg.org/#dom-domtokenlist-add
    ExceptionOr<void> Add(const List<DOMString> &tokens) noexcept
    {
      for (auto &token : tokens)
      {
        if (auto result = ValidateToken(token); result.HasException())
        {
          return result.ReleaseException();
        }
      }

      for (auto &token : tokens)
      {
        if (!std::ranges::contains(_tokens, token))
        {
          _tokens.emplace_back(token);
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
      for (auto &token : tokens)
      {
        if (auto result = ValidateToken(token); result.HasException())
        {
          return result.ReleaseException();
        }
      }

      auto it = std::ranges::remove_if(_tokens, [&](const DOMString &current)
                                       { return std::ranges::contains(tokens, current); });
      _tokens.erase(it.begin(), it.end());

      UpdateSteps();

      return {};
    }

    /// @see https://dom.spec.whatwg.org/#dom-domtokenlist-remove
    ExceptionOr<void> Remove(const DOMString &token) noexcept
    {
      return Remove(List<DOMString> {token});
    }

    /// @see https://dom.spec.whatwg.org/#dom-domtokenlist-toggle
    ExceptionOr<bool> Toggle(const DOMString &token, Maybe<bool> force = Null) noexcept
    {
      if (auto result = ValidateToken(token); result.HasException())
      {
        return result.ReleaseException();
      }

      if (std::ranges::contains(_tokens, token))
      {
        if (!force.value_or(false))
        {
          auto it =
            std::ranges::remove_if(_tokens, [&](const DOMString &current) { return current == token; });
          _tokens.erase(it.begin(), it.end());

          UpdateSteps();
          return false;
        }
        return true;
      }

      if (force.value_or(true))
      {
        _tokens.emplace_back(token);
        UpdateSteps();
        return true;
      }

      return false;
    }

    /// @see https://dom.spec.whatwg.org/#dom-domtokenlist-replace
    ExceptionOr<bool> Replace(const DOMString &token, const DOMString &newToken) noexcept
    {
      if (auto result = ValidateToken(token); result.HasException())
      {
        return result.ReleaseException();
      }

      if (auto result = ValidateToken(newToken); result.HasException())
      {
        return result.ReleaseException();
      }

      if (!std::ranges::contains(_tokens, token))
      {
        return false;
      }

      std::ranges::replace(_tokens, token, newToken);
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
      return SerializeSteps();
    }

    /// @see https://dom.spec.whatwg.org/#dom-domtokenlist-value
    void Value(DOMString &&value) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-domtokenlist-iterable
    KRYS_NODISCARD iterator begin() noexcept
    {
      return _tokens.begin();
    }

    /// @see https://dom.spec.whatwg.org/#dom-domtokenlist-iterable
    KRYS_NODISCARD iterator end() noexcept
    {
      return _tokens.end();
    }

    /// @see https://dom.spec.whatwg.org/#dom-domtokenlist-iterable
    KRYS_NODISCARD const_iterator begin() const noexcept
    {
      return _tokens.begin();
    }

    /// @see https://dom.spec.whatwg.org/#dom-domtokenlist-iterable
    KRYS_NODISCARD const_iterator end() const noexcept
    {
      return _tokens.end();
    }

#pragma endregion

  private:
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
    KRYS_NODISCARD DOMString SerializeSteps() const noexcept;

    KRYS_NODISCARD ExceptionOr<void> ValidateToken(DOMStringView token) const noexcept
    {
      if (token.empty())
      {
        return Exception {ExceptionCode::SyntaxError};
      }

      if (std::ranges::any_of(token, &StringAlgorithms::IsASCIIWhitespace))
      {
        return Exception {ExceptionCode::InvalidCharacterError};
      }

      return {};
    }
  };
}