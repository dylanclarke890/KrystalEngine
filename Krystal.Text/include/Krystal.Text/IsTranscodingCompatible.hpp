#pragma once

#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Text/CodePoint.hpp"
#include "Krystal.Text/CodeUnit.hpp"
#include "Krystal.Text/Encodings/ASCII.hpp"
#include "Krystal.Text/Encodings/UTF8.hpp"
#include "Krystal.Text/IsUnicodeCodePoint.hpp"

namespace Krys
{
  namespace Impl
  {
    // clang-format off
		template <typename TFrom, typename TTo>
		concept IsBitwiseTranscodingCompatible =
			SameType<remove_cvref_t<TTo>, remove_cvref_t<TFrom>>
			// if the To is from ASCII, then it's bitwise-compatible with all UTF-8 implementations,
			// provided those UTF-8 implementations have a sufficiently sized/aligned char that can be bit-copied.
			|| (SameType<remove_cvref_t<TFrom>, ascii_t>
				&& (SameType<remove_cvref_t<TTo>, utf8_t>
					|| DerivedFrom<Impl::utf8_tag, remove_cvref_t<TTo>>)
				&& ((sizeof(code_unit_t<remove_cvref_t<TTo>>) == sizeof(char))
					&& (alignof(code_unit_t<remove_cvref_t<TTo>>) == alignof(char)))
			)
		;
    // clang-format on
  }

  /// @brief Checks whether or not the specified `TFrom` encoding can be transcoded to the `TTo` encoding
  /// without by form of bit copying.
  /// @tparam TFrom The encoding that is going to decode the input code units into the intermediate code
  /// points.
  /// @tparam TTo The encoding that is going to encode the intermediate code points into the final code
  /// units.
  template <typename TFrom, typename TTo>
  concept IsBitwiseTranscodingCompatible =
    Impl::IsBitwiseTranscodingCompatible<remove_cvref_t<TFrom>, remove_cvref_t<TTo>>;

  namespace Impl
  {
    // clang-format off
		template <typename TFrom, typename TTo>
		concept IsTranscodingCompatible
			= IsBitwiseTranscodingCompatible<TFrom,TTo>
			|| SameType<remove_cvref_t<TFrom>, remove_cvref_t<TTo>>
			|| SameType<code_point_t<TFrom>, code_point_t<TTo>>
			|| (IsUnicodeScalarValue<code_point_t<TFrom>>
				? (IsUnicodeCodePoint<code_point_t<TTo>>)
				: (IsUnicodeCodePoint<TFrom> && !IsUnicodeScalarValue<TTo>));
    // clang-format on
  }

  /// @brief Checks whether or not the specified `TFrom` encoding can be transcoded to the `TTo` encoding
  /// without invoking a lossy conversion when using the intermediate code points.
  /// @tparam TFrom The encoding that is going to decode the input code units into the intermediate code
  /// points.
  /// @tparam TTo The encoding that is going to encode the intermediate code points into the final code
  /// units.
  /// @remarks First, it checks if the encodings are bitwise compatible with one another (e.g., transcoding
  /// ASCII to UTF-8). If that is not the case, then it checks if the two encodings are just identical.
  /// Finally, it checks if the code point types are the same or if it's putting unicode scalar values into
  /// unicode code points (which is valid one way, but not the other way since scalar values do not allow
  /// surrogates). If none of these are true, then, the intermediate code point likely cannot convert
  /// between the two losslessly.
  template <typename TFrom, typename TTo>
  concept IsTranscodingCompatible = Impl::IsTranscodingCompatible<TFrom, TTo>;
}
