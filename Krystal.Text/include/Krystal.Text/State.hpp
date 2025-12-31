#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"

namespace Krys
{
  /// @brief Whether or not a state has an `IsComplete()` method.
  template <typename TState>
  concept HasStateIsCompleteMethod = requires(TState &state) {
    { state.IsComplete() } -> ConvertibleTo<bool>;
  };

  /// @brief Whether or not an encoding has an `IsStateComplete(state)` method.
  template <typename TEncoding, typename TState>
  concept HasEncodingIsStateCompleteMethod = requires(TEncoding &encoding, TState &state) {
    { encoding.IsStateComplete(state) } -> ConvertibleTo<bool>;
  };

  /// @brief Whether or not a state is capable of outputting data even after an input is fully processed.
  /// @tparam TEncoding The encoding type to test.
  /// @tparam TState The state type to test.
  /// @remarks This means that there is either an `encoding.IsStateComplete(state)` call that is
  /// well-formed, or a `state.IsComplete()` call that is well-formed.
  template <typename TEncoding, typename TState>
  concept IsStateOutputCapable =
    HasStateIsCompleteMethod<TState> || HasEncodingIsStateCompleteMethod<TEncoding, TState>;

  /// @brief Whether or not the given type can be constructed without information from the encoding
  /// itself.
  /// @tparam TEncoding The encoding that may contain necessary information.
  /// @tparam TState The state type that may need information from the encoding to be successfully
  /// constructed.
  /// @remarks This value tells users at compile time whether or not they need to be careful with the state.
  template <typename TEncoding, typename TState>
  concept IsStateIndependant = !IsConstructible<TState, TEncoding> && DefaultConstructible<TState>;

  /// @brief Whether or not the encoding's `DecodeState` can be constructed without information from the
  /// encoding itself.
  template <typename TEncoding>
  concept IsDecodeStateIndependant = IsStateIndependant<TEncoding, DecodeState<TEncoding>>;

  /// @brief Whether or not the encoding's `EncodeState` can be constructed without information from the
  /// encoding itself.
  template <typename TEncoding>
  concept IsEncodeStateIndependant = IsStateIndependant<TEncoding, EncodeState<TEncoding>>;

  /// @brief Retrieves the `DecodeState` of an encoding type if it exists; otherwise, retrieves the general
  /// `State` type.
  template <typename TEncoding>
  using DecodeState =
    typename Conditional<TEncoding::DecodeState, typename TEncoding::DecodeState, typename TEncoding::State>;

  /// @brief Retrieves the `EncodeState` of an encoding type if it exists; otherwise, retrieves the general
  /// `State` type.
  template <typename TEncoding>
  using EncodeState =
    typename Conditional<TEncoding::EncodeState, typename TEncoding::EncodeState, typename TEncoding::State>;

  struct EmptyState
  {
  };

  /// @brief Returns whether or not a state has completed any associated operations and has no more
  /// manipulations on the output to perform, even if the input source is empty.
  /// @param[in] state The state to check for completion.
  /// @remarks If the state does not have a member function `IsComplete()`, then this will simply return
  /// `true`. Otherwise, it invokes `state.IsComplete()`.
  template <typename TState>
  KRYS_NODISCARD constexpr bool IsStateComplete(KRYS_MAYBE_UNUSED TState &state) noexcept
  {
    if constexpr (HasStateIsCompleteMethod<TState &>)
    {
      return state.IsComplete();
    }
    else
    {
      return true;
    }
  }

  /// @brief Returns whether or not a state has completed any associated operations and has no more
  /// manipulations on the output to perform, even if the input source is empty.
  /// @param[in] encoding The encoding for the state.
  /// @param[in] state The state to check for completion.
  /// @remarks If the state does not have a member function `IsComplete`, then this will simply return
  /// `true`. Otherwise, it invokes `state.IsComplete()`.
  template <typename TEncoding, typename TState>
  KRYS_NODISCARD constexpr bool IsStateComplete(KRYS_MAYBE_UNUSED TEncoding &encoding, TState &state) noexcept
  {
    if constexpr (HasEncodingIsStateCompleteMethod<TState &>)
    {
      return encoding.IsStateComplete(state);
    }
    else
    {
      return IsStateComplete(state);
    }
  }

  /// @brief Constructs the `DecodeState` of the given encoding, based on whether or not the encoding and
  /// state meet the criteria of IsDecodeStateIndependant.
  /// @param[in] encoding The encoding object to use, if applicable, for the construction of the state.
  template <typename TEncoding>
  KRYS_NODISCARD constexpr auto CreateDecodeState(KRYS_MAYBE_UNUSED TEncoding &encoding) noexcept
  {
    using encoding_t = RemoveCvRef<TEncoding>;
    using state_t = DecodeState<encoding_t>;

    if constexpr (IsDecodeStateIndependant<encoding_t>)
    {
      return state_t {};
    }
    else
    {
      return state_t {encoding};
    }
  }

  /// @brief Constructs the `DecodeState` of the given encoding, based on whether or not the encoding and
  /// state meet the criteria of IsDecodeStateIndependant or whether it can be created by copy
  /// construction from the given `encodeState`.
  /// @param[in] encoding The encoding object to use, if applicable, for the construction of the state.
  /// @param[in] encodeState A preexisting state from the encoder.
  template <typename TEncoding>
  KRYS_NODISCARD constexpr auto
    CreateDecodeStateWith(KRYS_MAYBE_UNUSED TEncoding &encoding,
                          KRYS_MAYBE_UNUSED const EncodeState<RemoveCvRef<TEncoding>> &encodeState) noexcept
  {
    using encoding_t = RemoveCvRef<TEncoding>;
    using decode_state_t = DecodeState<encoding_t>;
    using encode_state_t = EncodeState<encoding_t>;

    if constexpr (IsDecodeStateIndependant<encoding_t>)
    {
      if constexpr (IsConstructible<decode_state_t, const encode_state_t &>)
      {
        return decode_state_t {encodeState};
      }
      else
      {
        return decode_state_t {};
      }
    }
    else
    {
      if constexpr (IsConstructible<decode_state_t, const encoding_t &, const encode_state_t &>)
      {
        return decode_state_t {encoding, encodeState};
      }
      else
      {
        return decode_state_t {encoding};
      }
    }
  }

  /// @brief Constructs the `DecodeState` of the given encoding, based on whether or not the encoding and
  /// state meet the criteria of IsDecodeStateIndependant or whether it can be created by copy
  /// construction from the given `encodeState`.
  /// @param[in] encoding The encoding object to use, if applicable, for the construction of the state.
  /// @param[in] decodeState A preexisting state from the decoder.
  template <typename TEncoding>
  KRYS_NODISCARD constexpr auto
    CopyDecodeStateWith(KRYS_MAYBE_UNUSED TEncoding &encoding,
                        KRYS_MAYBE_UNUSED const DecodeState<RemoveCvRef<TEncoding>> &decodeState) noexcept
  {
    using encoding_t = RemoveCvRef<TEncoding>;
    using decode_state_t = DecodeState<encoding_t>;

    if constexpr (IsDecodeStateIndependant<encoding_t>)
    {
      if constexpr (IsConstructible<decode_state_t, const decode_state_t &>)
      {
        return decode_state_t {decodeState};
      }
      else
      {
        return decode_state_t {};
      }
    }
    else
    {
      if constexpr (IsConstructible<decode_state_t, const encoding_t &, const decode_state_t &>)
      {
        return decode_state_t {encoding, decodeState};
      }
      else
      {
        return decode_state_t {encoding};
      }
    }
  }

  /// @brief Constructs the `EncodeState` of the given encoding, based on whether or not the encoding and
  /// state meet the criteria of IsEncodeStateIndependant.
  /// @param[in] encoding The encoding object to use, if applicable, for the construction of the state.
  template <typename TEncoding>
  KRYS_NODISCARD constexpr auto CreateEncodeState(KRYS_MAYBE_UNUSED TEncoding &encoding) noexcept
  {
    using encoding_t = RemoveCvRef<TEncoding>;
    using state_t = EncodeState<encoding_t>;

    if constexpr (IsEncodeStateIndependant<encoding_t>)
    {
      return state_t {};
    }
    else
    {
      return state_t {encoding};
    }
  }

  /// @brief Constructs the `EncodeState` of the given encoding, based on whether or not the encoding and
  /// state meet the criteria of IsEncodeStateIndependant or whether it can be created by copy
  /// construction from the given `decodeState`.
  /// @param[in] encoding The encoding object to use, if applicable, for the construction of the state.
  /// @param[in] decodeState A preexisting state from the decoder.
  template <typename TEncoding>
  KRYS_NODISCARD constexpr auto
    CreateEncodeStateWith(KRYS_MAYBE_UNUSED TEncoding &encoding,
                          KRYS_MAYBE_UNUSED const DecodeState<RemoveCvRef<TEncoding>> &decodeState) noexcept
  {
    using encoding_t = RemoveCvRef<TEncoding>;
    using decode_state_t = DecodeState<encoding_t>;
    using encode_state_t = EncodeState<encoding_t>;

    if constexpr (IsEncodeStateIndependant<encoding_t>)
    {
      if constexpr (IsConstructible<encode_state_t, const decode_state_t &>)
      {
        return encode_state_t {decodeState};
      }
      else
      {
        return encode_state_t {};
      }
    }
    else
    {
      if constexpr (IsConstructible<encode_state_t, const encoding_t &, const decode_state_t &>)
      {
        return encode_state_t {encoding, decodeState};
      }
      else
      {
        return encode_state_t {encoding};
      }
    }
  }

  /// @brief Constructs the `EncodeState` of the given encoding, based on whether or not the encoding and
  /// state meet the criteria of IsEncodeStateIndependant or whether it can be created by copy
  /// construction from the given `encodeState`.
  /// @param[in] encoding The encoding object to use, if applicable, for the construction of the state.
  /// @param[in] encodeState A preexisting state from the encoder to attempt to copy.
  template <typename TEncoding>
  KRYS_NODISCARD constexpr auto
    CopyEncodeStateWith(KRYS_MAYBE_UNUSED TEncoding &encoding,
                        KRYS_MAYBE_UNUSED const EncodeState<RemoveCvRef<TEncoding>> &encodeState) noexcept
  {
    using encoding_t = RemoveCvRef<TEncoding>;
    using encode_state_t = EncodeState<encoding_t>;

    if constexpr (IsEncodeStateIndependant<encoding_t>)
    {
      if constexpr (IsConstructible<encode_state_t, const encode_state_t &>)
      {
        return encode_state_t {encodeState};
      }
      else
      {
        return encode_state_t {};
      }
    }
    else
    {
      if constexpr (IsConstructible<encode_state_t, const encoding_t &, const encode_state_t &>)
      {
        return encode_state_t {encoding, encodeState};
      }
      else
      {
        return encode_state_t {encoding};
      }
    }
  }
}