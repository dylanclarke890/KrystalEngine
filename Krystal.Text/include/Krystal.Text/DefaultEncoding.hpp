#pragma once

#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Text/Encodings/NoEncoding.hpp"
#include "Krystal.Text/Encodings/UTF16.hpp"
#include "Krystal.Text/Encodings/UTF32.hpp"
#include "Krystal.Text/Encodings/UTF8.hpp"
#include "Krystal.Text/EncodingScheme.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"
#include "Krystal.Text/UnicodeScalarValue.hpp"
#include <cstdint>

namespace Krys
{
  namespace Impl
  {
    template <typename T, bool IsCompileTime>
    class DefaultCodeUnitEncoding
    {
    private:
        // clang-format off
      using TChosen = conditional_t<SameType<T, char>, ascii_t,
        conditional_t<SameType<T, wchar>, basic_utf16<wchar>,
          conditional_t<SameType<T, schar>, basic_ascii<schar>,
            conditional_t<SameType<T, uchar8>, utf8_t,
              conditional_t<SameType<T, uchar>, basic_utf8<uchar>,
                conditional_t<SameType<T, char8>, basic_utf8<char8>,
                  conditional_t<SameType<T, char16>, utf16_t,
                    conditional_t<SameType<T, char32>, utf32_t,
                      conditional_t<SameType<T, UnicodeCodePoint>, basic_utf32<UnicodeCodePoint>,
                        conditional_t<SameType<T, UnicodeScalarValue>, basic_utf32<UnicodeScalarValue>,
                          conditional_t<SameType<T, byte>, EncodingScheme<utf8_t, Endian::System, byte>,
                                        basic_no_encoding<T, UnicodeCodePoint>>>>>>>>>>>>;
      // clang-format on

      static_assert(!::Krys::IsSpecializationOf<TChosen, basic_no_encoding>,
                    "there is no default encoding for the given code unit type");

    public:
      /// @brief The chosen type for the given code unit.
      /// @remarks The default encodings for code unit types are as follows
      /// - `char` ➡ ascii_t
      /// - `char8_t` ➡ utf8
      /// - `ztd::uchar8_t` ➡ utf8_t (if different from `char8_t` type)
      /// - `std::byte` ➡ basic_utf8<std::byte>
      /// - `signed` char ➡ basic_ascii<signed char>
      /// - `char16_t` ➡ utf16
      /// - `char32_t` ➡ utf32
      /// - `unicode_code_point` ➡ utf32_t (if different from `char32_t` type)
      /// - `unicode_scalar_value` ➡ utf32_t (if different from `char32_t` type)
      using type = TChosen;
    };

    template <typename T, bool>
    class DefaultCodePointEncoding
    {
    private:
      static_assert(IsUnicodeCodePoint<T> || SameType<T, char32_t>,
                    "there is no default encoding for the given code point type");

    public:
      /// @brief The chosen type for the given code unit.
      /// @remarks The default encodings for code point types are as follows
      /// - `char32_t` ➡ utf8
      /// - `unicode_code_point` ➡ utf8
      /// - `unicode_scalar_value` ➡ utf8
      using type = utf8_t;
    };
  }

  /// @brief The default encoding associated with a given code unit type, that serves as either input to a
  /// decode operation or output from an encode operation.
  /// @tparam T The code unit type, with no cv-qualifiers
  template <typename T>
  class DefaultCodeUnitEncoding : public Impl::DefaultCodeUnitEncoding<T, false>
  {
  };

  /// @brief A `typename` alias for DefaultCodeUnitEncoding.
  /// @tparam T The code unit type, with no cv-qualifiers
  template <typename T>
  using default_code_unit_encoding_t = typename DefaultCodeUnitEncoding<T>::type;

  /// @brief The default encoding associated with a given code unit type, that serves as either input to a
  /// decode operation or output from an encode operation. This uses the additional information that this is
  /// compiletime, not runtime, to help make the decision on what to do.
  /// @tparam T The code unit type, with no cv-qualifiers
  template <typename T>
  class DefaultConstevalCodeUnitEncoding : public Impl::DefaultCodeUnitEncoding<T, true>
  {
  };

  /// @brief A `typename` alias for DefaultConstevalCodeUnitEncoding.
  /// @tparam T The code unit type, with no cv-qualifiers
  template <typename T>
  using default_consteval_code_unit_encoding_t = typename DefaultConstevalCodeUnitEncoding<T>::type;

  /// @brief The default encoding associated with a given code point type, that serves as either input to an
  /// encode operation or output from decode operation.
  /// @tparam T The code point type, with no cv-qualifiers
  template <typename T>
  class DefaultCodePointEncoding : public Impl::DefaultCodePointEncoding<T, false>
  {
  };

  /// @brief A `typename` alias for DefaultCodePointEncoding.
  /// @tparam T The code point type, with no cv-qualifiers
  template <typename T>
  using default_code_point_encoding_t = typename DefaultCodePointEncoding<T>::type;

  /// @brief The default encoding associated with a given code point type, that serves as either input to an
  /// encode operation or output from decode operation. This uses additional information that this is at
  /// compile time, not run time, to help make a decision as to what to do.
  /// @tparam T The code point type, with no cv-qualifiers
  template <typename T>
  class DefaultConstevalCodePointEncoding : public Impl::DefaultCodePointEncoding<T, true>
  {
  };

  /// @brief A `typename` alias for DefaultConstevalCodePointEncoding.
  /// @tparam T The code point type, with no cv-qualifiers
  template <typename T>
  using default_consteval_code_point_encoding_t = typename DefaultConstevalCodePointEncoding<T>::type;
}