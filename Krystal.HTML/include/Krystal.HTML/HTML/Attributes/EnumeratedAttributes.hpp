#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"
#include "Krystal.HTML/Types/DOMStringAtom.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/String/FixedString.hpp"
#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"
#include "Krystal.Lib/Types/Span.hpp"

namespace Krys::HTML::Attributes
{
  template <IsEnum TEnum>
  struct KeywordStateMapping
  {
    DOMStringAtom Keyword;
    TEnum State;
  };

  template <FixedString AttributeName, DerivedFrom<HTMLElement> TElement>
  struct EnumeratedAttributeTraits
  {
    using StateEnum = void;
    //
    // Must have:
    // using StateEnum = TStateEnum;
    //
    // Must have:
    // static inline Array<KeywordStateMapping<StateEnum>, 1> KeywordToStateMappings = { KeywordStateMapping
    // {u8"ltr", StateEnum::LTR} };
    //
    // As needed:
    // static inline Array<KeywordStateMapping<StateEnum>, 1> StateToCanonicalKeywordMappings = {
    // KeywordStateMapping {u8"ltr", StateEnum::LTR} };
    //
    // As needed:
    // constexpr static StateEnum MissingValueDefault = StateEnum::RTL;
    //
    // As needed:
    // constexpr static StateEnum InvalidValueDefault = StateEnum::LTR;
    //
    // As needed:
    // constexpr static StateEnum EmptyValueDefault = StateEnum::Auto;
  };

  template <typename Traits>
  concept HasValidEnumeratedAttributeTraits = !Void<typename Traits::StateEnum>;

  template <typename Traits>
  concept HasKeywordToStateMappings = requires { Traits::KeywordToStateMappings; };

  template <typename Traits>
  concept HasMissingValueDefault = requires { Traits::MissingValueDefault; };

  template <typename Traits>
  concept HasStateToCanonicalKeywordMappings = requires { Traits::StateToCanonicalKeywordMappings; };

  template <typename Traits>
  concept HasInvalidValueDefault = requires { Traits::InvalidValueDefault; };

  template <typename Traits>
  concept HasEmptyValueDefault = requires { Traits::EmptyValueDefault; };

  template <FixedString AttributeName, DerivedFrom<HTMLElement> TElement>
  class EnumeratedAttribute
  {
    using Traits = EnumeratedAttributeTraits<AttributeName, TElement>;

    static_assert(HasValidEnumeratedAttributeTraits<Traits>,
                  "Must define 'EnumeratedAttributeTraits' for this AttributeName/TElement.");

    static_assert(HasKeywordToStateMappings<Traits>,
                  "Enumerated attributes must have keyword to state mappings defined.");

  public:
    template <OneOf<DOMString, Maybe<DOMString>> TContentAttributeValue>
    KRYS_NODISCARD static TContentAttributeValue ResolveCanonicalKeyword(Maybe<DOMString> &&value) noexcept
    {
      auto state = ResolveState(value);
      if (state == Null)
      {
        if constexpr (SameType<TContentAttributeValue, Maybe<DOMString>>)
        {
          return Null;
        }
        else
        {
          return DOMString {};
        }
      }

      auto canonicalKeyword = ResolveCanonicalKeywordForState(*state);
      if constexpr (SameType<TContentAttributeValue, Maybe<DOMString>>)
      {
        return canonicalKeyword == DOMStringAtom::Null() ? Null : DOMString(canonicalKeyword.View());
      }
      else if constexpr (SameType<TContentAttributeValue, DOMString>)
      {
        return canonicalKeyword == DOMStringAtom::Null() ? DOMString {} : DOMString(canonicalKeyword.View());
      }
    }

  private:
    KRYS_NODISCARD static Maybe<typename Traits::StateEnum>
      ResolveState(const Maybe<DOMString> &contentAttributeValue) noexcept
    {
      if (contentAttributeValue == Null)
      {
        if constexpr (HasMissingValueDefault<Traits>)
        {
          return Traits::MissingValueDefault;
        }

        return Null;
      }

      auto &value = *contentAttributeValue;

      auto keywordMappings = Traits::KeywordToStateMappings;

      auto it =
        std::find_if(keywordMappings.begin(), keywordMappings.end(), [&](auto &mapping)
                     { return StringAlgorithms::ASCIICaseInsensitiveMatch(value, mapping.Keyword.View()); });

      if (it != keywordMappings.end())
      {
        return it->State;
      }

      if constexpr (HasEmptyValueDefault<Traits>)
      {
        if (value.empty())
        {
          return Traits::EmptyValueDefault;
        }
      }

      if constexpr (HasInvalidValueDefault<Traits>)
      {
        return Traits::InvalidValueDefault;
      }

      return Null;
    }

    KRYS_NODISCARD static DOMStringAtom
      ResolveCanonicalKeywordForState(typename Traits::StateEnum state) noexcept
    {
      if constexpr (HasStateToCanonicalKeywordMappings<Traits>)
      {
        auto &stateToKeywordMappings = Traits::StateToCanonicalKeywordMappings;
        auto it = std::find_if(stateToKeywordMappings.begin(), stateToKeywordMappings.end(),
                               [&](auto &mapping) { return mapping.State == state; });

        if (it != stateToKeywordMappings.end())
        {
          return it->Keyword;
        }
      }
      else
      {
        auto &keywordToStateMappings = Traits::KeywordToStateMappings;
        auto it = std::find_if(keywordToStateMappings.begin(), keywordToStateMappings.end(),
                               [&](auto &mapping) { return mapping.State == state; });

        if (it != keywordToStateMappings.end())
        {
          return it->Keyword;
        }
      }

      return DOMStringAtom::Null();
    }
  };
}

#pragma region EnumeratedAttributeTraits

#pragma region Dir Attribute

namespace Krys::HTML::Attributes
{
  enum class DirState : uint8
  {
    Undefined,
    LTR,
    RTL,
    Auto
  };

  template <>
  struct EnumeratedAttributeTraits<"dir", HTMLElement>
  {
    using StateEnum = DirState;

    static inline Array<KeywordStateMapping<DirState>, 3> KeywordToStateMappings = {
      KeywordStateMapping {u8"ltr", DirState::LTR}, KeywordStateMapping {u8"rtl", DirState::RTL},
      KeywordStateMapping {u8"auto", DirState::Auto}};

    constexpr static StateEnum MissingValueDefault = StateEnum::Undefined;
    constexpr static StateEnum InvalidValueDefault = StateEnum::Undefined;
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::Attributes::DirState, 4uz);

#pragma endregion

#pragma endregion