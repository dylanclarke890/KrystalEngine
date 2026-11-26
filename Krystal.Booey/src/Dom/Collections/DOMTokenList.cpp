#include "Krystal.Booey/Dom/Collections/DOMTokenList.hpp"
#include "Krystal.Booey/Dom/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/Dom/Algorithms/OrderedSet.hpp"
#include "Krystal.Booey/Dom/Nodes/Attr.hpp"
#include "Krystal.Booey/Dom/Nodes/Element.hpp"
#include "Krystal.Booey/Html/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Core/Move.hpp"
#include "Krystal.Core/Utils/SetForScope.hpp"

namespace krys::boo
{
  DOMTokenList::DOMTokenList(Element &element, DOMStringAtom attributeName,
                             IsSupportedTokenFunction isSupportedToken) noexcept
      : _attributeName {krys::move(attributeName)}, _element {ShareCheckedRef(element)},
        _isSupportedToken {krys::move(isSupportedToken)}
  {
    UpdateTokensFromAttributeValue();
  }

  UniquePtr<DOMTokenList> DOMTokenList::Create(Element &element, DOMStringAtom attributeName,
                                               IsSupportedTokenFunction isSupportedToken) noexcept
  {
    return UniquePtr<DOMTokenList>(
      new DOMTokenList(element, krys::move(attributeName), krys::move(isSupportedToken)));
  }

#pragma region DOMTokenList - https://dom.spec.whatwg.org/#domtokenlist

  bool DOMTokenList::Contains(const DOMString &token) const noexcept
  {
    return std::ranges::contains(Tokens(), token);
  }

  ExceptionOr<void> DOMTokenList::Add(const List<DOMString> &tokens) noexcept
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

  ExceptionOr<void> DOMTokenList::Remove(const List<DOMString> &tokens) noexcept
  {
    html::CEReactionsScope ceReactions;
    // 1. For each token of tokens:
    for (auto &token : tokens)
    {
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

  ExceptionOr<bool> DOMTokenList::Toggle(const DOMString &token, Maybe<bool> force) noexcept
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
      // 3a. If force is either not given or is false, then remove token from this’s token set, run the update
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
    //    and return true.
    if (force.value_or(true))
    {
      Tokens().emplace_back(token);
      UpdateSteps();
      return true;
    }

    // 5. Return false.
    return false;
  }

  ExceptionOr<bool> DOMTokenList::Replace(const DOMString &token, const DOMString &newToken) noexcept
  {
    html::CEReactionsScope ceReactions;

    // 1. If either token or newToken is the empty string, then throw a "SyntaxError" DOMException.
    if (token.empty() || newToken.empty())
    {
      return Exception {ExceptionCode::SyntaxError};
    }

    // 2. If either token or newToken contains any ASCII whitespace, then throw an "InvalidCharacterError"
    // DOMException.
    if (std::ranges::any_of(token, &infra::StringAlgorithms::IsASCIIWhitespace)
        || std::ranges::any_of(newToken, &infra::StringAlgorithms::IsASCIIWhitespace))
    {
      return Exception {ExceptionCode::InvalidCharacterError};
    }

    // 3. If this’s token set does not contain token, then return false.
    auto tokenIt = std::ranges::find(Tokens(), token);
    if (tokenIt == Tokens().end())
    {
      return false;
    }

    // 4. Replace token in this’s token set with newToken.
    auto newTokenIt = std::ranges::find(Tokens(), newToken);
    if (newTokenIt == Tokens().end())
    {
      *tokenIt = newToken;
    }
    // The first of `token` and `newToken` becomes `newToken`, and the other is removed.
    else if (newTokenIt > tokenIt)
    {
      *tokenIt = newToken;
      Tokens().erase(newTokenIt);
    }
    else if (newTokenIt < tokenIt)
    {
      Tokens().erase(tokenIt);
    }

    // 5. Run the update steps.
    UpdateSteps();

    // 6. Return true.
    return true;
  }

  void DOMTokenList::Value(DOMString value) noexcept
  {
    html::CEReactionsScope ceReactions;
    dom::ElementAlgorithms::SetAttributeValue(*_element, _attributeName, krys::move(value));
  }

#pragma endregion

  void DOMTokenList::UpdateTokensFromAttributeValue() const noexcept
  {
    auto tokens =
      dom::OrderedSet::Parser(dom::ElementAlgorithms::GetAttributeValue(*_element, _attributeName));
    _tokens.clear();
    _tokens.append_range(tokens);
    _tokensNeedUpdating = false;
  }

  ExceptionOr<bool> DOMTokenList::ValidationSteps(DOMStringView token) const noexcept
  {
    // 1. If set’s element and attribute name does not define supported tokens, then throw a TypeError.
    if (!_isSupportedToken)
    {
      return Exception {ExceptionCode::TypeError};
    }

    // 2. Let lowercaseToken be token, in ASCII lowercase.
    auto lowercaseToken = krys::text::ToASCIILower(token);

    // 3. If lowercaseToken is present in the supported tokens of set’s element and attribute name, then
    // return true.
    // 4. Return false.
    return _isSupportedToken(_element->NodeDocument(), lowercaseToken);
  }

  void DOMTokenList::UpdateSteps() noexcept
  {
    // 1. If get an attribute by namespace and local name given null, set’s attribute name, and set’s element
    //    returns null and set’s token set is empty, then return.
    auto *attr = dom::ElementAlgorithms::GetAttributeByNamespace(null, _attributeName, *_element);
    if (attr == nullptr && _tokens.empty())
    {
      return;
    }

    // 2. Set an attribute value given set’s element, set’s attribute name, and the result of running the
    //    ordered set serializer for set’s token set.
    SetForScope inAttributeUpdate(_inUpdateAssociatedAttributeFromTokens, true);
    dom::ElementAlgorithms::SetAttributeValue(*_element, _attributeName,
                                              dom::OrderedSet::Serialiser(_tokens));
  }

  DOMString DOMTokenList::SerialiseSteps() const noexcept
  {
    return dom::ElementAlgorithms::GetAttributeValue(*_element, _attributeName);
  }

  ExceptionOr<void> DOMTokenList::ValidateToken(DOMStringView token) noexcept
  {
    // 1a. If token is the empty string, then throw a "SyntaxError" DOMException.
    if (token.empty())
    {
      return Exception {ExceptionCode::SyntaxError};
    }

    // 1b. If token contains any ASCII whitespace, then throw an "InvalidCharacterError" DOMException.
    if (std::ranges::any_of(token, &infra::StringAlgorithms::IsASCIIWhitespace))
    {
      return Exception {ExceptionCode::InvalidCharacterError};
    }

    return {};
  }
}
