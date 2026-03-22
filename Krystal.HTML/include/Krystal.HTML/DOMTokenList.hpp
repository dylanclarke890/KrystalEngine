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

  class DOMTokenList
  {
  private:
    ReferenceWrapper<const QualifiedName> _attributeName;
    SmallList<DOMString, 1> _tokens;
    CheckedRef<Element> _element;
    IsSupportedTokenFunction _isSupportedToken;

  public:
    DOMTokenList(Element &element, const QualifiedName &attributeName,
                 IsSupportedTokenFunction &&isSupportedToken = {}) noexcept;

    KRYS_NODISCARD size_t Length() const noexcept
    {
      return _tokens.size();
    }

    KRYS_NODISCARD Maybe<DOMString> Item(size_t index) noexcept
    {
      if (index >= _tokens.size())
      {
        return std::nullopt;
      }

      return _tokens[index];
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
    }

    ExceptionOr<void> Add(const DOMString &token) noexcept
    {
      return Add({token});
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
    }

    ExceptionOr<void> Remove(const DOMString &token) noexcept
    {
      return Remove({token});
    }

    ExceptionOr<bool> Toggle(const DOMString &token, Maybe<bool> force = std::nullopt) noexcept
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

    void Value(const DOMString &value) noexcept;

  private:
    /// @see https://dom.spec.whatwg.org/#concept-domtokenlist-validation
    KRYS_NODISCARD ExceptionOr<bool> ValidationSteps(DOMStringView token) const noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-dtl-update
    KRYS_NODISCARD void UpdateSteps() noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-dtl-serialize
    KRYS_NODISCARD DOMString SerializeSteps() const noexcept;

    KRYS_NODISCARD ExceptionOr<void> ValidateToken(DOMStringView token)
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