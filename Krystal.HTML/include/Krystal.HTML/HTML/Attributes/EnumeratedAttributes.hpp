#pragma once

#include "Krystal.HTML/HTML/Enums/AttributeState.hpp"
#include "Krystal.HTML/HTML/HTMLElement.hpp"
#include "Krystal.HTML/DOM/Types/DOMString.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"
#include "Krystal.Lib/Types/NTTPString.hpp"
#include "Krystal.Lib/Types/Span.hpp"

namespace Krys::HTML::Attributes
{
  struct KeywordStateMapping
  {
    DOMStringAtom Keyword = DOMStringAtom::Null();
    AttributeState State;
  };

  struct StateKeywordMapping
  {
    AttributeState State;
    DOMStringAtom Keyword = DOMStringAtom::Null();
  };

  KRYS_NODISCARD inline KeywordStateMapping TrueStateMapping() noexcept
  {
    return KeywordStateMapping {DOMStringAtom(u8"true"), AttributeState::True};
  }

  KRYS_NODISCARD inline KeywordStateMapping FalseStateMapping() noexcept
  {
    return KeywordStateMapping {DOMStringAtom(u8"false"), AttributeState::False};
  }

  KRYS_NODISCARD inline KeywordStateMapping YesStateMapping() noexcept
  {
    return KeywordStateMapping {DOMStringAtom(u8"yes"), AttributeState::Yes};
  }

  KRYS_NODISCARD inline KeywordStateMapping NoStateMapping() noexcept
  {
    return KeywordStateMapping {DOMStringAtom(u8"no"), AttributeState::No};
  }

  KRYS_NODISCARD inline KeywordStateMapping OnStateMapping() noexcept
  {
    return KeywordStateMapping {DOMStringAtom(u8"on"), AttributeState::On};
  }

  KRYS_NODISCARD inline KeywordStateMapping OffStateMapping() noexcept
  {
    return KeywordStateMapping {DOMStringAtom(u8"off"), AttributeState::Off};
  }

  KRYS_NODISCARD inline KeywordStateMapping NoneStateMapping() noexcept
  {
    return KeywordStateMapping {DOMStringAtom(u8"none"), AttributeState::None};
  }

  KRYS_NODISCARD inline KeywordStateMapping AutoStateMapping() noexcept
  {
    return KeywordStateMapping {DOMStringAtom(u8"auto"), AttributeState::Auto};
  }

  KRYS_NODISCARD inline KeywordStateMapping DefaultStateMapping() noexcept
  {
    return KeywordStateMapping {DOMStringAtom(u8"default"), AttributeState::Default};
  }

  KRYS_NODISCARD inline KeywordStateMapping InheritStateMapping() noexcept
  {
    return KeywordStateMapping {DOMStringAtom(u8"inherit"), AttributeState::Inherit};
  }

  template <size_t N>
  using KeywordStateMappingArray = Array<KeywordStateMapping, N>;

  template <size_t N>
  using StateKeywordMappingArray = Array<StateKeywordMapping, N>;

  /// @brief Traits struct that must be specialized for each enumerated attribute on each element it applies
  /// to (or just for `HTMLElement` in the case of generic enumerated attributes).
  /// Must have:
  /// @code
  /// static inline KeywordStateMappingArray<1> StateMappings = { KeywordStateMapping {u8"ltr",
  /// AttributeState::LTR} };
  ///
  /// As needed:
  /// static inline KeywordStateMappingArray<1> CanonicalMappings = { KeywordStateMapping {u8"ltr",
  /// AttributeState::LTR} };
  ///
  /// As needed:
  /// constexpr static AttributeState MissingValueDefault = AttributeState::RTL;
  ///
  /// As needed:
  /// constexpr static AttributeState InvalidValueDefault = AttributeState::LTR;
  ///
  /// As needed:
  /// constexpr static AttributeState EmptyValueDefault = AttributeState::Auto;
  template <NTTPString AttributeName, DerivedFrom<HTMLElement> TElement>
  struct EnumeratedAttributeTraits;

  template <typename Traits>
  concept HasStateMappings = requires { Traits::StateMappings; };

  template <typename Traits>
  concept HasValidEnumeratedAttributeTraits = HasStateMappings<Traits>;

  template <typename Traits>
  concept HasMissingValueDefault = requires { Traits::MissingValueDefault; };

  template <typename Traits>
  concept HasCanonicalMappings = requires { Traits::CanonicalMappings; };

  template <typename Traits>
  concept HasInvalidValueDefault = requires { Traits::InvalidValueDefault; };

  template <typename Traits>
  concept HasEmptyValueDefault = requires { Traits::EmptyValueDefault; };

  template <typename Traits>
  concept CanReturnNullResolvedState =
    !HasMissingValueDefault<Traits> || (!HasInvalidValueDefault<Traits> && !HasEmptyValueDefault<Traits>);

  template <NTTPString AttributeName, DerivedFrom<HTMLElement> TElement>
  class EnumeratedAttribute
  {
    using Traits = EnumeratedAttributeTraits<AttributeName, TElement>;

    static_assert(HasValidEnumeratedAttributeTraits<Traits>,
                  "Must define 'EnumeratedAttributeTraits' for this AttributeName/TElement.");

    static_assert(HasStateMappings<Traits>,
                  "Enumerated attributes must have keyword to state mappings defined.");

  public:
    template <OneOf<DOMString, Maybe<DOMString>> TContentAttributeValue>
    KRYS_NODISCARD static TContentAttributeValue ResolveCanonicalKeyword(Maybe<DOMString> &&value) noexcept
    {
      auto state = ResolveState(value);
      if constexpr (CanReturnNullResolvedState<Traits>)
      {
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
      }

      auto canonicalKeyword = [&]()
      {
        if constexpr (CanReturnNullResolvedState<Traits>)
        {
          return ResolveCanonicalKeywordForState(*state);
        }
        else
        {
          return ResolveCanonicalKeywordForState(state);
        }
      }();

      if constexpr (SameType<TContentAttributeValue, Maybe<DOMString>>)
      {
        return canonicalKeyword == DOMStringAtom::Null() ? Null : DOMString(canonicalKeyword.View());
      }
      else if constexpr (SameType<TContentAttributeValue, DOMString>)
      {
        return canonicalKeyword == DOMStringAtom::Null() ? DOMString {} : DOMString(canonicalKeyword.View());
      }
    }

    KRYS_NODISCARD static conditional_t<CanReturnNullResolvedState<Traits>, Maybe<AttributeState>,
                                        AttributeState>
      ResolveState(const Maybe<DOMString> &contentAttributeValue) noexcept
    {
      if (contentAttributeValue == Null)
      {
        if constexpr (HasMissingValueDefault<Traits>)
        {
          return Traits::MissingValueDefault;
        }

        if constexpr (CanReturnNullResolvedState<Traits>)
        {
          return Null;
        }
      }

      auto &value = *contentAttributeValue;

      auto keywordMappings = Traits::StateMappings;

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

      if constexpr (CanReturnNullResolvedState<Traits>)
      {
        return Null;
      }
    }

    KRYS_NODISCARD static DOMStringAtom ResolveCanonicalKeywordForState(AttributeState state) noexcept
    {
      if constexpr (HasCanonicalMappings<Traits>)
      {
        auto &canonicalMappings = Traits::CanonicalMappings;
        auto it = std::find_if(canonicalMappings.begin(), canonicalMappings.end(),
                               [&](auto &mapping) { return mapping.State == state; });

        if (it != canonicalMappings.end())
        {
          return it->Keyword;
        }
      }
      else
      {
        auto &keywordToStateMappings = Traits::StateMappings;
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

  template <>
  struct EnumeratedAttributeTraits<"dir", HTMLElement>
  {
    static inline KeywordStateMappingArray<3> StateMappings = {
      KeywordStateMapping {u8"ltr", AttributeState::LTR}, KeywordStateMapping {u8"rtl", AttributeState::RTL},
      AutoStateMapping()};

    constexpr static AttributeState MissingValueDefault = AttributeState::Undefined;
    constexpr static AttributeState InvalidValueDefault = AttributeState::Undefined;
  };

  template <>
  struct EnumeratedAttributeTraits<"translate", HTMLElement>
  {
    static inline KeywordStateMappingArray<2> StateMappings = {YesStateMapping(), NoStateMapping()};

    constexpr static AttributeState MissingValueDefault = AttributeState::Inherit;
    constexpr static AttributeState InvalidValueDefault = AttributeState::Inherit;
    constexpr static AttributeState EmptyValueDefault = AttributeState::Yes;
  };

  template <>
  struct EnumeratedAttributeTraits<"hidden", HTMLElement>
  {
    static inline KeywordStateMappingArray<2> StateMappings = {
      KeywordStateMapping {u8"hidden", AttributeState::Hidden},
      KeywordStateMapping {u8"until-found", AttributeState::UntilFound}};

    constexpr static AttributeState MissingValueDefault = AttributeState::NotHidden;
    constexpr static AttributeState InvalidValueDefault = AttributeState::Hidden;
    constexpr static AttributeState EmptyValueDefault = AttributeState::Hidden;
  };

  template <>
  struct EnumeratedAttributeTraits<"draggable", HTMLElement>
  {
    static inline KeywordStateMappingArray<2> StateMappings = {TrueStateMapping(), FalseStateMapping()};

    constexpr static AttributeState MissingValueDefault = AttributeState::Auto;
    constexpr static AttributeState InvalidValueDefault = AttributeState::Auto;
  };

  template <>
  struct EnumeratedAttributeTraits<"spellcheck", HTMLElement>
  {
    static inline KeywordStateMappingArray<2> StateMappings = {TrueStateMapping(), FalseStateMapping()};

    constexpr static AttributeState MissingValueDefault = AttributeState::Default;
    constexpr static AttributeState InvalidValueDefault = AttributeState::Default;
    constexpr static AttributeState EmptyValueDefault = AttributeState::True;
  };

  template <>
  struct EnumeratedAttributeTraits<"writingsuggestions", HTMLElement>
  {
    static inline KeywordStateMappingArray<2> StateMappings = {TrueStateMapping(), FalseStateMapping()};

    constexpr static AttributeState MissingValueDefault = AttributeState::Default;
    constexpr static AttributeState InvalidValueDefault = AttributeState::Default;
    constexpr static AttributeState EmptyValueDefault = AttributeState::True;
  };

  template <>
  struct EnumeratedAttributeTraits<"autocapitalize", HTMLElement>
  {
    static inline KeywordStateMappingArray<6> StateMappings = {
      KeywordStateMapping {u8"off", AttributeState::None},
      KeywordStateMapping {u8"none", AttributeState::None},
      KeywordStateMapping {u8"on", AttributeState::Sentences},
      KeywordStateMapping {u8"sentences", AttributeState::Sentences},
      KeywordStateMapping {u8"words", AttributeState::Words},
      KeywordStateMapping {u8"characters", AttributeState::Characters},
    };

    static inline StateKeywordMappingArray<5> CanonicalMappings = {
      StateKeywordMapping {AttributeState::Default, u8""},
      StateKeywordMapping {AttributeState::None, u8"none"},
      StateKeywordMapping {AttributeState::Sentences, u8"sentences"},
      StateKeywordMapping {AttributeState::Words, u8"words"},
      StateKeywordMapping {AttributeState::Characters, u8"characters"},
    };

    constexpr static AttributeState MissingValueDefault = AttributeState::Default;
    constexpr static AttributeState InvalidValueDefault = AttributeState::Sentences;
  };

  template <>
  struct EnumeratedAttributeTraits<"autocorrect", HTMLElement>
  {
    static inline KeywordStateMappingArray<2> StateMappings = {OnStateMapping(), OffStateMapping()};

    constexpr static AttributeState MissingValueDefault = AttributeState::On;
    constexpr static AttributeState InvalidValueDefault = AttributeState::On;
    constexpr static AttributeState EmptyValueDefault = AttributeState::On;
  };

  template <>
  struct EnumeratedAttributeTraits<"fetchpriority", HTMLElement>
  {
    static inline KeywordStateMappingArray<3> StateMappings = {
      KeywordStateMapping {u8"low", AttributeState::Low},
      KeywordStateMapping {u8"high", AttributeState::High},
      KeywordStateMapping {u8"auto", AttributeState::Auto},
    };

    constexpr static AttributeState MissingValueDefault = AttributeState::Auto;
    constexpr static AttributeState InvalidValueDefault = AttributeState::Auto;
  };

  template <>
  struct EnumeratedAttributeTraits<"crossorigin", HTMLElement>
  {
    static inline KeywordStateMappingArray<3> StateMappings = {
      KeywordStateMapping {u8"anonymous", AttributeState::Anonymous},
      KeywordStateMapping {u8"use-credentials", AttributeState::UseCredentials},
    };

    constexpr static AttributeState MissingValueDefault = AttributeState::NoCORS;
    constexpr static AttributeState InvalidValueDefault = AttributeState::Anonymous;
    constexpr static AttributeState EmptyValueDefault = AttributeState::Anonymous;
  };
}
