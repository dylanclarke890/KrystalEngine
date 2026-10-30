#pragma once

#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo::css
{
  struct SizesCalcValue
  {
    double Value {0};
    bool IsLength {false};
    char32 Operation {0};

    SizesCalcValue() noexcept = default;
    SizesCalcValue(double numericValue, bool length) noexcept : Value(numericValue), IsLength(length)
    {
    }
  };

  /// @brief Evaluates the `calc()` of a `sizes` attribute, before style is resolved: lengths resolve against
  /// the initial style, and only `+`, `-`, `*`, `/` and parentheses are allowed.
  class SizesCalcParser
  {
  private:
    List<SizesCalcValue> _valueList;
    float _result {0};
    bool _isValid {false};
    const dom::Document &_document;

  public:
    SizesCalcParser(TokenRange range, const dom::Document &document) noexcept;

    KRYS_NODISCARD float Result() const noexcept;
    KRYS_NODISCARD bool IsValid() const noexcept
    {
      return _isValid;
    }

  private:
    KRYS_NODISCARD bool CalcToReversePolishNotation(TokenRange range) noexcept;
    KRYS_NODISCARD bool Calculate() noexcept;
    void AppendNumber(const Token &token) noexcept;
    KRYS_NODISCARD bool AppendLength(const Token &token) noexcept;
    KRYS_NODISCARD bool HandleOperator(List<Token> &stack, const Token &token) noexcept;
    void AppendOperator(const Token &token) noexcept;
  };
}
