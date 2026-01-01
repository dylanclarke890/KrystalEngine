#pragma once

#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Text/IsUnicodeEncoding.hpp"
#include "Krystal.Text/TextEncodingId.hpp"

namespace Krys
{
  namespace Impl
  {
    template <typename TFromEncoding, typename TToEncoding>
    inline constexpr bool DecodeSameAsEncode =
      decoded_id_v<TFromEncoding> == encoded_id_v<TToEncoding>        // cf
      && decoded_id_v<TFromEncoding> == decoded_id_v<TToEncoding>     // cf
      && decoded_id_v<TFromEncoding> != Krys::TextEncodingId::unknown // cf
      && encoded_id_v<TToEncoding> != Krys::TextEncodingId::unknown;

    template <typename TFromEncoding, typename TToEncoding>
    inline constexpr bool AlreadyDecoded =
      encoded_id_v<TFromEncoding> == decoded_id_v<TToEncoding>        // cf
      && decoded_id_v<TFromEncoding> != Krys::TextEncodingId::unknown // cf
      && encoded_id_v<TToEncoding> != Krys::TextEncodingId::unknown;

    template <typename TFromEncoding, typename TToEncoding>
    inline constexpr bool EncodeSameAsDecode =
      encoded_id_v<TFromEncoding> == decoded_id_v<TToEncoding>        // cf
      && encoded_id_v<TFromEncoding> == encoded_id_v<TToEncoding>     // cf
      && encoded_id_v<TFromEncoding> != Krys::TextEncodingId::unknown // cf
      && decoded_id_v<TToEncoding> != Krys::TextEncodingId::unknown;

    template <typename TFromEncoding, typename TToEncoding>
    inline constexpr bool AlreadyEncoded =
      decoded_id_v<TFromEncoding> == encoded_id_v<TToEncoding>        // cf
      && encoded_id_v<TFromEncoding> != Krys::TextEncodingId::unknown // cf
      && decoded_id_v<TToEncoding> != Krys::TextEncodingId::unknown;
  }

  /// @brief Whether or not the decode encoding is
  template <typename TFrom, typename TTo>
  concept IsDecodeRedundant = SameType<remove_cvref_t<TFrom>, remove_cvref_t<TTo>>;

  /// @brief Whether or not the decode encoding is
  template <typename TFrom, typename TTo>
  concept IsEncodeRedundant = SameType<remove_cvref_t<TFrom>, remove_cvref_t<TTo>>;
}
