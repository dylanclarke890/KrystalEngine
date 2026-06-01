#pragma once

#include "Krystal.HTML/HTML/MicroParsers/MicroParserError.hpp"

namespace Krys::HTML::MicroParsers
{
  /// @brief Represents the result of a micro-parsing operation, containing either a successfully parsed value
  /// or an error.
  /// @note Comparison operators only compare the `Value` field, the `Error` field is not considered for
  /// comparison. Ensure that the `Error` field is checked separately to determine the success of the parsing
  /// operation.
  template <typename T>
  struct MicroParserResult
  {
    T Value {};
    MicroParserError Error {MicroParserError::None};

    KRYS_NODISCARD bool Success() const noexcept
    {
      return Error == MicroParserError::None;
    }

    friend bool operator<=(const MicroParserResult &lhs, const MicroParserResult &rhs) noexcept
    {
      return lhs.Value <= rhs.Value;
    }

    friend bool operator<(const MicroParserResult &lhs, const MicroParserResult &rhs) noexcept
    {
      return lhs.Value < rhs.Value;
    }

    friend bool operator>=(const MicroParserResult &lhs, const MicroParserResult &rhs) noexcept
    {
      return lhs.Value >= rhs.Value;
    }

    friend bool operator>(const MicroParserResult &lhs, const MicroParserResult &rhs) noexcept
    {
      return lhs.Value > rhs.Value;
    }

    friend bool operator==(const MicroParserResult &lhs, const MicroParserResult &rhs) noexcept
    {
      return lhs.Value == rhs.Value;
    }

    friend bool operator!=(const MicroParserResult &lhs, const MicroParserResult &rhs) noexcept
    {
      return lhs.Value != rhs.Value;
    }

    friend bool operator<=(const MicroParserResult &lhs, const T& rhs) noexcept
    {
      return lhs.Value <= rhs;
    }

    friend bool operator<(const MicroParserResult &lhs, const T &rhs) noexcept
    {
      return lhs.Value < rhs;
    }

    friend bool operator>=(const MicroParserResult &lhs, const T &rhs) noexcept
    {
      return lhs.Value >= rhs;
    }

    friend bool operator>(const MicroParserResult &lhs, const T &rhs) noexcept
    {
      return lhs.Value > rhs;
    }

    friend bool operator==(const MicroParserResult &lhs, const T &rhs) noexcept
    {
      return lhs.Value == rhs;
    }

    friend bool operator!=(const MicroParserResult &lhs, const T &rhs) noexcept
    {
      return lhs.Value != rhs;
    }
  };
}