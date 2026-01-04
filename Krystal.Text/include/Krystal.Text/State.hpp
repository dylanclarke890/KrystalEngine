#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"

namespace Krys::detail::state
{
  template <typename TEncoding, typename = void>
  struct decode_state
  {
    using type = typename TEncoding::state;
  };

  template <typename TEncoding>
  struct decode_state<TEncoding, void_t<typename TEncoding::decode_state>>
  {
    using type = typename TEncoding::decode_state;
  };

  template <typename TEncoding, typename = void>
  struct encode_state
  {
    using type = typename TEncoding::state;
  };

  template <typename TEncoding>
  struct encode_state<TEncoding, void_t<typename TEncoding::encode_state>>
  {
    using type = typename TEncoding::encode_state;
  };
}

namespace Krys
{
  /// @brief A default empty state type for encodings that do not require any state information.
  struct EmptyState
  {
  };

  /// @brief Retrieves the `decode_state` of an encoding type if it exists; otherwise, retrieves the general
  /// `state` type.
  template <typename TEncoding>
  using decode_state_t = ::Krys::detail::state::decode_state<remove_cvref_t<TEncoding>>::type;

  /// @brief Retrieves the `encode_state` of an encoding type if it exists; otherwise, retrieves the general
  /// `state` type.
  template <typename TEncoding>
  using encode_state_t = ::Krys::detail::state::encode_state<remove_cvref_t<TEncoding>>::type;

  template <typename TState>
  concept StateHasIsCompleteMethod = requires(TState &state) {
    { state.IsComplete() } -> ConvertibleTo<bool>;
  };

  /// @brief Returns whether or not a state has completed any associated operations and has no more
  /// manipulations on the output to perform, even if the input source is empty.
  /// @param[in] state The state to check for completion.
  /// @remarks Returns true if the state does not have a `IsComplete()` method.
  template <typename TState>
  KRYS_NODISCARD constexpr bool IsStateComplete(KRYS_MAYBE_UNUSED TState &state) noexcept
  {
    if constexpr (StateHasIsCompleteMethod<TState &>)
    {
      return state.IsComplete();
    }
    else
    {
      return true;
    }
  }

  template <typename TEncoding, typename TState>
  concept EncodingHasIsStateCompleteMethod = requires(TEncoding &encoding, TState &state) {
    { encoding.IsStateComplete(state) } -> ConvertibleTo<bool>;
  };

  /// @brief Returns whether or not a state has completed any associated operations and has no more
  /// manipulations on the output to perform, even if the input source is empty.
  /// @param[in] encoding The encoding for the state.
  /// @param[in] state The state to check for completion.
  /// @remarks Returns true if the state does not have a `IsComplete()` method and the encoding does not have
  /// an `IsStateComplete(state)` method.
  template <typename TEncoding, typename TState>
  KRYS_NODISCARD constexpr bool IsStateComplete(KRYS_MAYBE_UNUSED TEncoding &encoding, TState &state) noexcept
  {
    if constexpr (EncodingHasIsStateCompleteMethod<TEncoding &, TState &>)
    {
      return encoding.IsStateComplete(state);
    }
    else
    {
      return ::Krys::IsStateComplete(state);
    }
  }

  /// @brief Whether the given type can be constructed without information from the encoding itself.
  /// @tparam TEncoding The encoding that may contain necessary information.
  /// @tparam TState The state type that may need information from the encoding to be successfully
  /// constructed.
  /// @remarks This value tells users at compile time whether or not they need to be careful with the state.
  template <typename TEncoding, typename TState>
  concept IsStateIndependant =
    !Constructible<TState, remove_cvref_t<TEncoding>> && DefaultConstructible<TState>;

  /// @brief Whether or not the encoding's `decode_state` can be constructed without information from the
  /// encoding itself.
  template <typename TEncoding>
  concept IsDecodeStateIndependant = IsStateIndependant<TEncoding, decode_state_t<TEncoding>>;

  /// @brief Whether or not the encoding's `encode_state` can be constructed without information from the
  /// encoding itself.
  template <typename TEncoding>
  concept IsEncodeStateIndependant = IsStateIndependant<TEncoding, encode_state_t<TEncoding>>;

  /// @brief Constructs the `decode_state` of the given encoding, based on whether or not the encoding and
  /// state meet the criteria of IsDecodeStateIndependant.
  /// @param[in] encoding The encoding object to use, if applicable, for the construction of the state.
  template <typename TEncoding>
  KRYS_NODISCARD constexpr auto CreateDecodeState(KRYS_MAYBE_UNUSED TEncoding &encoding) noexcept
  {
    using TState = decode_state_t<TEncoding>;
    if constexpr (IsDecodeStateIndependant<TEncoding>)
    {
      return TState {};
    }
    else
    {
      return TState {encoding};
    }
  }

  /// @brief Constructs the `decode_state` of the given encoding, based on whether or not the encoding and
  /// state meet the criteria of IsDecodeStateIndependant or whether it can be created by copy
  /// construction from the given `state`.
  /// @param[in] encoding The encoding object to use, if applicable, for the construction of the state.
  /// @param[in] state A preexisting state from the encoder.
  template <typename TEncoding>
  KRYS_NODISCARD constexpr auto
    CreateDecodeStateWith(KRYS_MAYBE_UNUSED TEncoding &encoding,
                          KRYS_MAYBE_UNUSED const encode_state_t<TEncoding> &state) noexcept
  {
    using TDecodeState = decode_state_t<TEncoding>;
    using TEncodeState = encode_state_t<TEncoding>;

    if constexpr (IsDecodeStateIndependant<TEncoding>)
    {
      if constexpr (Constructible<TDecodeState, const TEncodeState &>)
      {
        return TDecodeState {state};
      }
      else
      {
        return TDecodeState {};
      }
    }
    else
    {
      if constexpr (Constructible<TDecodeState, const remove_cvref_t<TEncoding> &, const TEncodeState &>)
      {
        return TDecodeState {encoding, state};
      }
      else
      {
        return TDecodeState {encoding};
      }
    }
  }

  /// @brief Constructs the `decode_state` of the given encoding, based on whether or not the encoding and
  /// state meet the criteria of IsDecodeStateIndependant or whether it can be created by copy
  /// construction from the given `state`.
  /// @param[in] encoding The encoding object to use, if applicable, for the construction of the state.
  /// @param[in] state A preexisting state from the decoder.
  template <typename TEncoding>
  KRYS_NODISCARD constexpr auto
    CopyDecodeStateWith(KRYS_MAYBE_UNUSED TEncoding &encoding,
                        KRYS_MAYBE_UNUSED const decode_state_t<TEncoding> &state) noexcept
  {
    using TState = decode_state_t<TEncoding>;
    if constexpr (IsDecodeStateIndependant<TEncoding>)
    {
      if constexpr (Constructible<TState, const TState &>)
      {
        return TState {state};
      }
      else
      {
        return TState {};
      }
    }
    else
    {
      if constexpr (Constructible<TState, const remove_cvref_t<TEncoding> &, const TState &>)
      {
        return TState {encoding, state};
      }
      else
      {
        return TState {encoding};
      }
    }
  }

  /// @brief Constructs the `encode_state` of the given encoding, based on whether or not the encoding and
  /// state meet the criteria of IsEncodeStateIndependant.
  /// @param[in] encoding The encoding object to use, if applicable, for the construction of the state.
  template <typename TEncoding>
  KRYS_NODISCARD constexpr auto CreateEncodeState(KRYS_MAYBE_UNUSED TEncoding &encoding) noexcept
  {
    using TState = encode_state_t<TEncoding>;
    if constexpr (IsEncodeStateIndependant<TEncoding>)
    {
      return TState {};
    }
    else
    {
      return TState {encoding};
    }
  }

  /// @brief Constructs the `encode_state` of the given encoding, based on whether or not the encoding and
  /// state meet the criteria of IsEncodeStateIndependant or whether it can be created by copy
  /// construction from the given `decodeState`.
  /// @param[in] encoding The encoding object to use, if applicable, for the construction of the state.
  /// @param[in] decodeState A preexisting state from the decoder.
  template <typename TEncoding>
  KRYS_NODISCARD constexpr auto
    CreateEncodeStateWith(KRYS_MAYBE_UNUSED TEncoding &encoding,
                          KRYS_MAYBE_UNUSED const decode_state_t<TEncoding> &state) noexcept
  {
    using TDecodeState = decode_state_t<TEncoding>;
    using TEncodeState = encode_state_t<TEncoding>;
    if constexpr (IsEncodeStateIndependant<TEncoding>)
    {
      if constexpr (Constructible<TEncodeState, const TDecodeState &>)
      {
        return TEncodeState {state};
      }
      else
      {
        return TEncodeState {};
      }
    }
    else
    {
      if constexpr (Constructible<TEncodeState, const remove_cvref_t<TEncoding> &, const TDecodeState &>)
      {
        return TEncodeState {encoding, state};
      }
      else
      {
        return TEncodeState {encoding};
      }
    }
  }

  /// @brief Constructs the `encode_state` of the given encoding, based on whether or not the encoding and
  /// state meet the criteria of IsEncodeStateIndependant or whether it can be created by copy
  /// construction from the given `state`.
  /// @param[in] encoding The encoding object to use, if applicable, for the construction of the state.
  /// @param[in] state A preexisting state from the encoder to attempt to copy.
  template <typename TEncoding>
  KRYS_NODISCARD constexpr auto
    CopyEncodeStateWith(KRYS_MAYBE_UNUSED TEncoding &encoding,
                        KRYS_MAYBE_UNUSED const encode_state_t<TEncoding> &state) noexcept
  {
    using TState = encode_state_t<TEncoding>;
    if constexpr (IsEncodeStateIndependant<TEncoding>)
    {
      if constexpr (Constructible<TState, const TState &>)
      {
        return TState {state};
      }
      else
      {
        return TState {};
      }
    }
    else
    {
      if constexpr (Constructible<TState, const remove_cvref_t<TEncoding> &, const TState &>)
      {
        return TState {encoding, state};
      }
      else
      {
        return TState {encoding};
      }
    }
  }
}