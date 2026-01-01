#pragma once

/// @brief Check if a particular config parameter is enabled.
#define KRYS_CONFIG(PARAMETER) (defined KRYS_CONFIG_##PARAMETER && KRYS_CONFIG_##PARAMETER)

/// KRYS_CONFIG(STD_LIBRARY_TO_ADDRESS) - Whether to use std::to_address from the standard library if
/// available.
#ifndef KRYS_CONFIG_STD_LIBRARY_TO_ADDRESS
  #define KRYS_CONFIG_STD_LIBRARY_TO_ADDRESS 1
#endif

/// KRYS_CONFIG(STD_LIBRARY_CONTIGUOUS_ITERATOR_TAG) - Whether to use std::contiguous_iterator_tag from the
/// standard library if available.
#ifndef KRYS_CONFIG_STD_LIBRARY_CONTIGUOUS_ITERATOR_TAG
  #define KRYS_CONFIG_STD_LIBRARY_CONTIGUOUS_ITERATOR_TAG 1
#endif

/// KRYS_CONFIG(STD_LIBRARY_RANGES_BASIC_CONST_ITERATOR) - Whether to use std::basic_const_iterator from
/// the standard library if available.
#ifndef KRYS_CONFIG_STD_LIBRARY_RANGES_BASIC_CONST_ITERATOR
  #define KRYS_CONFIG_STD_LIBRARY_RANGES_BASIC_CONST_ITERATOR 1
#endif

/// KRYS_CONFIG(STD_LIBRARY_RANGES) - Whether to use the C++20 Ranges library from the standard library if
/// available.
#ifndef KRYS_CONFIG_STD_LIBRARY_RANGES
  #define KRYS_CONFIG_STD_LIBRARY_RANGES 1
#endif

/// KRYS_CONFIG(STD_LIBRARY_RANGES_REVERSE_CPOS) - Whether to use the C++20 Ranges library from the standard
/// library if available.
#ifndef KRYS_CONFIG_STD_LIBRARY_RANGES_REVERSE_CPOS
  #define KRYS_CONFIG_STD_LIBRARY_RANGES_REVERSE_CPOS 1
#endif

/// KRYS_CONFIG(NATIVE_CHAR8_T) - Whether the compiler has native support for char8_t.
#ifndef KRYS_CONFIG_NATIVE_CHAR8_T
  #define KRYS_CONFIG_NATIVE_CHAR8_T 1
#endif

/// KRYS_CONFIG(STD_LIBRARY_BORROWED_RANGE) - Whether to use std::ranges::borrowed_range from the
/// standard library if available.
#ifndef KRYS_CONFIG_STD_LIBRARY_BORROWED_RANGE
  #define KRYS_CONFIG_STD_LIBRARY_BORROWED_RANGE 1
#endif

/// KRYS_CONFIG(STD_LIBRARY_DEBUG_ITERATORS) - Whether to enable debug iterators from the standard library if
/// available.
#ifndef KRYS_CONFIG_STD_LIBRARY_DEBUG_ITERATORS
  #define KRYS_CONFIG_STD_LIBRARY_DEBUG_ITERATORS 1
#endif

/// KRYS_CONFIG(STD_LIBRARY_IS_NOTHROW_CONVERTIBLE) - Whether to use std::is_nothrow_convertible from the
/// standard library if available.
#ifndef KRYS_CONFIG_STD_LIBRARY_IS_NOTHROW_CONVERTIBLE
  #define KRYS_CONFIG_STD_LIBRARY_IS_NOTHROW_CONVERTIBLE 1
#endif