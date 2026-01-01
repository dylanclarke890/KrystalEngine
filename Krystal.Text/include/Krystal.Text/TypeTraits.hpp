#pragma once

#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/Config.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys
{
  /// @brief Checks if the given type is one of the plain character types.
  template <typename T>
  class is_character : public IntegralConstant<bool, SameType<T, char> || SameType<T, wchar> ||
#if KRYS_CONFIG(NATIVE_CHAR8_T)
                                                       SameType<T, char8> ||
#endif
                                                       SameType<T, uchar> || SameType<T, schar>
                                                       || SameType<T, char16> || SameType<T, char32>>
  {
  };

  template <typename T>
  concept IsCharacter = is_character<T>::value;

  /// @brief Checks if the given type is one of the types that is used as a code unit (unnamed char, wchar,
  /// char8, char16, and char32).
  template <typename T>
  class is_code_unit : public IntegralConstant<bool, SameType<T, char> || SameType<T, wchar> ||
#if KRYS_CONFIG(NATIVE_CHAR8_T)
                                                       SameType<T, char8> ||
#endif
                                                       SameType<T, char16> || SameType<T, char32>>
  {
  };

  template <typename T>
  concept IsCodeUnit = is_code_unit<T>::value;

  /// @brief Checks if the given type is one of the types that is usable in the standard with the
  /// std::char_traits traits type that's used for std::string_view, std::string and others.
  template <typename T>
  class is_char_traitable : public IntegralConstant<bool, SameType<T, char> || SameType<T, wchar> ||
#if KRYS_CONFIG(NATIVE_CHAR8_T)
                                                            SameType<T, char8> ||
#endif
                                                            SameType<T, char16> || SameType<T, char32>>
  {
  };

  template <typename T>
  concept IsCharTraitable = is_char_traitable<T>::value;

  /// @brief Checks if the given type is a pointer, and that is pointers to a character type.
  template <typename T>
  class is_character_pointer
      : public IntegralConstant<bool, IsPointer<remove_cvref_t<T>>
                                        && IsCharacter<remove_pointer_t<remove_cvref_t<T>>>>
  {
  };

  template <typename T>
  concept IsCharacterPointer = is_character_pointer<T>::value;
}