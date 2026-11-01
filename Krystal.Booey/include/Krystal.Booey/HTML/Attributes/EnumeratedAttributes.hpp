#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/HTML/Enums/AttributeState.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/NTTPString.hpp"
#include "Krystal.Core/Types/Span.hpp"

namespace krys::boo::html
{
  struct KeywordStateMapping
  {
    dom::DOMStringAtom Keyword = dom::DOMStringAtom::Null();
    AttributeState State;
  };

  struct StateKeywordMapping
  {
    AttributeState State;
    dom::DOMStringAtom Keyword = dom::DOMStringAtom::Null();
  };

  KRYS_NODISCARD inline KeywordStateMapping TrueStateMapping() noexcept
  {
    return KeywordStateMapping {dom::DOMStringAtom(u8"true"), AttributeState::True};
  }

  KRYS_NODISCARD inline KeywordStateMapping FalseStateMapping() noexcept
  {
    return KeywordStateMapping {dom::DOMStringAtom(u8"false"), AttributeState::False};
  }

  KRYS_NODISCARD inline KeywordStateMapping YesStateMapping() noexcept
  {
    return KeywordStateMapping {dom::DOMStringAtom(u8"yes"), AttributeState::Yes};
  }

  KRYS_NODISCARD inline KeywordStateMapping NoStateMapping() noexcept
  {
    return KeywordStateMapping {dom::DOMStringAtom(u8"no"), AttributeState::No};
  }

  KRYS_NODISCARD inline KeywordStateMapping OnStateMapping() noexcept
  {
    return KeywordStateMapping {dom::DOMStringAtom(u8"on"), AttributeState::On};
  }

  KRYS_NODISCARD inline KeywordStateMapping OffStateMapping() noexcept
  {
    return KeywordStateMapping {dom::DOMStringAtom(u8"off"), AttributeState::Off};
  }

  KRYS_NODISCARD inline KeywordStateMapping NoneStateMapping() noexcept
  {
    return KeywordStateMapping {dom::DOMStringAtom(u8"none"), AttributeState::None};
  }

  KRYS_NODISCARD inline KeywordStateMapping AutoStateMapping() noexcept
  {
    return KeywordStateMapping {dom::DOMStringAtom(u8"auto"), AttributeState::Auto};
  }

  KRYS_NODISCARD inline KeywordStateMapping DefaultStateMapping() noexcept
  {
    return KeywordStateMapping {dom::DOMStringAtom(u8"default"), AttributeState::Default};
  }

  KRYS_NODISCARD inline KeywordStateMapping InheritStateMapping() noexcept
  {
    return KeywordStateMapping {dom::DOMStringAtom(u8"inherit"), AttributeState::Inherit};
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

  /// @brief Whether an attribute can be in no state: when it's missing without a missing value default, or
  /// invalid without an invalid value default.
  template <typename Traits>
  concept CanReturnNullResolvedState = !HasMissingValueDefault<Traits> || !HasInvalidValueDefault<Traits>;

  /// @see https://html.spec.whatwg.org/#enumerated-attribute
  template <NTTPString AttributeName, DerivedFrom<HTMLElement> TElement>
  class EnumeratedAttribute
  {
    using Traits = EnumeratedAttributeTraits<AttributeName, TElement>;

    static_assert(HasValidEnumeratedAttributeTraits<Traits>,
                  "Must define 'EnumeratedAttributeTraits' for this AttributeName/TElement.");

    static_assert(HasStateMappings<Traits>,
                  "Enumerated attributes must have keyword to state mappings defined.");

  public:
    template <OneOf<dom::DOMString, Maybe<dom::DOMString>> TContentAttributeValue>
    KRYS_NODISCARD static TContentAttributeValue
      ResolveCanonicalKeyword(Maybe<dom::DOMString> &&value) noexcept
    {
      auto state = ResolveState(value);
      if constexpr (CanReturnNullResolvedState<Traits>)
      {
        if (state == null)
        {
          if constexpr (SameType<TContentAttributeValue, Maybe<dom::DOMString>>)
          {
            return null;
          }
          else
          {
            return dom::DOMString {};
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

      if constexpr (SameType<TContentAttributeValue, Maybe<dom::DOMString>>)
      {
        return canonicalKeyword == dom::DOMStringAtom::Null()
                 ? Maybe<dom::DOMString>()
                 : Maybe<dom::DOMString>(dom::DOMString(canonicalKeyword.View()));
      }
      else if constexpr (SameType<TContentAttributeValue, dom::DOMString>)
      {
        return canonicalKeyword == dom::DOMStringAtom::Null() ? dom::DOMString {}
                                                              : dom::DOMString(canonicalKeyword.View());
      }
    }

    /// @brief The state of an attribute whose value is @p contentAttributeValue (null when it's not
    /// specified); no state is null.
    /// @see https://html.spec.whatwg.org/#empty-value-default
    KRYS_NODISCARD static conditional_t<CanReturnNullResolvedState<Traits>, Maybe<AttributeState>,
                                        AttributeState>
      ResolveState(const Maybe<dom::DOMString> &contentAttributeValue) noexcept
    {
      // 1. If the attribute is not specified:
      if (contentAttributeValue == null)
      {
        // 1a. If the attribute has a missing value default state defined, then return that missing value
        //     default state.
        if constexpr (HasMissingValueDefault<Traits>)
        {
          return Traits::MissingValueDefault;
        }
        // 1b. Otherwise, return no state.
        else
        {
          return null;
        }
      }

      auto &value = *contentAttributeValue;

      // 2. If the attribute's value is an ASCII case-insensitive match for one of the keywords defined for
      // the
      //    attribute, then return the state represented by that keyword.
      auto &keywordMappings = Traits::StateMappings;
      auto it = std::find_if(
        keywordMappings.begin(), keywordMappings.end(), [&](auto &mapping)
        { return infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, mapping.Keyword.View()); });
      if (it != keywordMappings.end())
      {
        return it->State;
      }

      // 3. If the attribute has an empty value default state defined and the attribute's value is the empty
      //    string, then return that empty value default state.
      if constexpr (HasEmptyValueDefault<Traits>)
      {
        if (value.empty())
        {
          return Traits::EmptyValueDefault;
        }
      }

      // 4. If the attribute has an invalid value default state defined, then return that invalid value
      // default
      //    state.
      if constexpr (HasInvalidValueDefault<Traits>)
      {
        return Traits::InvalidValueDefault;
      }
      // 5. Return no state.
      else
      {
        return null;
      }
    }

    /// @brief The canonical keyword for @p state: the attribute's `CanonicalMappings` when it gives them (the
    /// keywords the specification gives explicitly), otherwise the first keyword mapping to @p state.
    /// @note The first keyword is the canonical one when it's the only keyword for the state, the only
    /// conforming one, or the conforming one that isn't empty (where an empty keyword is an empty value
    /// default here), so `StateMappings` lists those first.
    /// @see https://html.spec.whatwg.org/#canonical-keyword
    KRYS_NODISCARD static dom::DOMStringAtom ResolveCanonicalKeywordForState(AttributeState state) noexcept
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

      return dom::DOMStringAtom::Null();
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

  /// @see https://html.spec.whatwg.org/#lazy-loading-attribute
  template <>
  struct EnumeratedAttributeTraits<"loading", HTMLElement>
  {
    static inline KeywordStateMappingArray<2> StateMappings = {
      KeywordStateMapping {u8"lazy", AttributeState::Lazy},
      KeywordStateMapping {u8"eager", AttributeState::Eager}};

    constexpr static AttributeState MissingValueDefault = AttributeState::Eager;
    constexpr static AttributeState InvalidValueDefault = AttributeState::Eager;
  };

  /// @brief A submit button's `formmethod`, which has no missing value default.
  /// @see https://html.spec.whatwg.org/#attr-fs-formmethod
  template <>
  struct EnumeratedAttributeTraits<"formmethod", HTMLElement>
  {
    static inline KeywordStateMappingArray<3> StateMappings = {
      KeywordStateMapping {u8"get", AttributeState::Get},
      KeywordStateMapping {u8"post", AttributeState::Post},
      KeywordStateMapping {u8"dialog", AttributeState::Dialog},
    };

    constexpr static AttributeState InvalidValueDefault = AttributeState::Get;
  };

  /// @brief A submit button's `formenctype`, which has no missing value default.
  /// @see https://html.spec.whatwg.org/#attr-fs-formenctype
  template <>
  struct EnumeratedAttributeTraits<"formenctype", HTMLElement>
  {
    static inline KeywordStateMappingArray<3> StateMappings = {
      KeywordStateMapping {u8"application/x-www-form-urlencoded", AttributeState::URLEncoded},
      KeywordStateMapping {u8"multipart/form-data", AttributeState::FormData},
      KeywordStateMapping {u8"text/plain", AttributeState::TextPlain},
    };

    constexpr static AttributeState InvalidValueDefault = AttributeState::URLEncoded;
  };

  /// @see https://html.spec.whatwg.org/#fetch-priority-attribute
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
  struct EnumeratedAttributeTraits<"popover", HTMLElement>
  {
    static inline KeywordStateMappingArray<3> StateMappings = {
      AutoStateMapping(), KeywordStateMapping {u8"manual", AttributeState::Manual},
      KeywordStateMapping {u8"hint", AttributeState::Hint}};

    // The missing value default is the No Popover state (null).
    constexpr static AttributeState InvalidValueDefault = AttributeState::Manual;
    constexpr static AttributeState EmptyValueDefault = AttributeState::Auto;
  };

  /// @see https://html.spec.whatwg.org/#cors-settings-attribute
  /// @see https://html.spec.whatwg.org/#attr-crossorigin-none
  template <>
  struct EnumeratedAttributeTraits<"crossorigin", HTMLElement>
  {
    static inline KeywordStateMappingArray<2> StateMappings = {
      KeywordStateMapping {u8"anonymous", AttributeState::Anonymous},
      KeywordStateMapping {u8"use-credentials", AttributeState::UseCredentials},
    };

    constexpr static AttributeState MissingValueDefault = AttributeState::NoCORS;
    constexpr static AttributeState InvalidValueDefault = AttributeState::Anonymous;
    constexpr static AttributeState EmptyValueDefault = AttributeState::Anonymous;
  };

  /// @brief A referrer policy attribute: its keywords are the referrer policies, and the empty string (the
  /// `Default` state here) is its missing and invalid value default.
  /// @see https://html.spec.whatwg.org/#referrer-policy-attribute
  template <>
  struct EnumeratedAttributeTraits<"referrerpolicy", HTMLElement>
  {
    static inline KeywordStateMappingArray<9> StateMappings = {
      KeywordStateMapping {u8"", AttributeState::Default},
      KeywordStateMapping {u8"no-referrer", AttributeState::NoReferrer},
      KeywordStateMapping {u8"no-referrer-when-downgrade", AttributeState::NoReferrerWhenDowngrade},
      KeywordStateMapping {u8"same-origin", AttributeState::SameOrigin},
      KeywordStateMapping {u8"origin", AttributeState::Origin},
      KeywordStateMapping {u8"strict-origin", AttributeState::StrictOrigin},
      KeywordStateMapping {u8"origin-when-cross-origin", AttributeState::OriginWhenCrossOrigin},
      KeywordStateMapping {u8"strict-origin-when-cross-origin", AttributeState::StrictOriginWhenCrossOrigin},
      KeywordStateMapping {u8"unsafe-url", AttributeState::UnsafeURL},
    };

    constexpr static AttributeState MissingValueDefault = AttributeState::Default;
    constexpr static AttributeState InvalidValueDefault = AttributeState::Default;
  };
}
