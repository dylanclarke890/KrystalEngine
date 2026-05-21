#pragma once

#include "Krystal.HTML/Algorithms/StringAlgorithms.hpp"
#include "Krystal.HTML/DOMString.hpp"
#include "Krystal.HTML/QualifiedName.hpp"
#include "Krystal.HTML/Utils/ExceptionOr.hpp"
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

  class DOMTokenList
  {
    friend class Element;
    friend class ExtensibilityHooks;
    friend class HTMLCollectionAlgorithms;

    using TokenList = SmallList<DOMString, 1>;

  private:
    DOMStringAtom _attributeName;
    TokenList _tokens;
    CheckedRef<Element> _element;
    IsSupportedTokenFunction _isSupportedToken;

  public:
    using iterator = TokenList::iterator;
    using const_iterator = TokenList::const_iterator;

    DOMTokenList(Element &element, DOMStringAtom attributeName,
                 IsSupportedTokenFunction &&isSupportedToken = DefaultIsSupportedTokenFunction) noexcept;

    KRYS_NODISCARD size_t Length() const noexcept
    {
      return _tokens.size();
    }

    KRYS_NODISCARD Maybe<DOMString> Item(size_t index) const noexcept
    {
      if (index >= _tokens.size())
      {
        return Null;
      }

      return _tokens[index];
    }

    KRYS_NODISCARD Maybe<DOMString> operator[](size_t index) const noexcept
    {
      return Item(index);
    }

    KRYS_NODISCARD bool Contains(const DOMString &token) const noexcept
    {
      return std::ranges::contains(_tokens, token);
    }

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

    ExceptionOr<void> Add(const DOMString &token) noexcept
    {
      return Add(List<DOMString> {token});
    }

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

    ExceptionOr<void> Remove(const DOMString &token) noexcept
    {
      return Remove(List<DOMString> {token});
    }

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

    KRYS_NODISCARD ExceptionOr<bool> Supports(DOMStringView token) const noexcept
    {
      return ValidationSteps(token);
    }

    KRYS_NODISCARD DOMString Value() const noexcept
    {
      return SerializeSteps();
    }

    void Value(DOMString &&value) noexcept;

    KRYS_NODISCARD iterator begin() noexcept
    {
      return _tokens.begin();
    }

    KRYS_NODISCARD iterator end() noexcept
    {
      return _tokens.end();
    }

    KRYS_NODISCARD const_iterator begin() const noexcept
    {
      return _tokens.begin();
    }

    KRYS_NODISCARD const_iterator end() const noexcept
    {
      return _tokens.end();
    }

  private:
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