#pragma once

#include "Krystal.Core/Maths/Base.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include <algorithm>
#include <ranges>

namespace krys
{
  template <typename, size_t>
  struct ColourComponents;

  template <size_t ColumnCount, size_t RowCount>
  class ColourMatrix
  {
  private:
    Array<float, RowCount * ColumnCount> _matrix;

  public:
    explicit constexpr ColourMatrix(Span<const float, RowCount * ColumnCount> s) noexcept
    {
      std::ranges::copy(s, _matrix.begin());
    }

    template <std::convertible_to<float>... Ts>
    explicit constexpr ColourMatrix(Ts... input) noexcept : _matrix {{static_cast<float>(input)...}}
    {
      static_assert(sizeof...(Ts) == RowCount * ColumnCount);
    }

    template <size_t NumberOfComponents>
    KRYS_NODISCARD constexpr ColourComponents<float, NumberOfComponents>
      TransformedColourComponents(const ColourComponents<float, NumberOfComponents> &) const noexcept;

    KRYS_NODISCARD constexpr float at(size_t row, size_t column) const noexcept
    {
      return _matrix[(row * ColumnCount) + column];
    }

    KRYS_NODISCARD const Array<float, RowCount * ColumnCount> &data() const noexcept
    {
      return _matrix;
    }

    friend bool operator==(const ColourMatrix &, const ColourMatrix &) = default;

    template <size_t ToColumnCount, size_t ToRowCount>
    constexpr operator ColourMatrix<ToColumnCount, ToRowCount>() const noexcept;
  };

  template <>
  template <>
  constexpr ColourMatrix<3, 3>::operator ColourMatrix<5, 4>() const noexcept
  {
    return ColourMatrix<5, 4> {at(0, 0), at(0, 1), at(0, 2), 0, 0, at(1, 0), at(1, 1), at(1, 2), 0, 0,
                               at(2, 0), at(2, 1), at(2, 2), 0, 0, 0,        0,        0,        1, 0};
  }

  KRYS_NODISCARD constexpr ColourMatrix<3, 3> BrightnessColourMatrix(float amount) noexcept
  {
    // Brightness is specified as a component transfer function:
    // https://www.w3.org/TR/filter-effects-1/#brightnessEquivalent which is equivalent to the following
    // matrix.
    amount = std::max(amount, 0.0f);
    return ColourMatrix<3, 3> {
      amount, 0.0f, 0.0f, 0.0f, amount, 0.0f, 0.0f, 0.0f, amount,
    };
  }

  KRYS_NODISCARD constexpr ColourMatrix<5, 4> ContrastColourMatrix(float amount) noexcept
  {
    // Contrast is specified as a component transfer function:
    // https://www.w3.org/TR/filter-effects-1/#contrastEquivalent which is equivalent to the following matrix.
    amount = std::max(amount, 0.0f);
    float intercept = -0.5f * amount + 0.5f;
    return ColourMatrix<5, 4> {amount, 0.0f, 0.0f,   0.0f, intercept, 0.0f, amount, 0.0f, 0.0f, intercept,
                               0.0f,   0.0f, amount, 0.0f, intercept, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f};
  }

  KRYS_NODISCARD constexpr ColourMatrix<3, 3> GrayscaleColourMatrix(float amount) noexcept
  {
    // Values from https://www.w3.org/TR/filter-effects-1/#grayscaleEquivalent
    float oneMinusAmount = std::clamp(1.0f - amount, 0.0f, 1.0f);
    return ColourMatrix<3, 3> {0.2126f + 0.7874f * oneMinusAmount, 0.7152f - 0.7152f * oneMinusAmount,
                               0.0722f - 0.0722f * oneMinusAmount, 0.2126f - 0.2126f * oneMinusAmount,
                               0.7152f + 0.2848f * oneMinusAmount, 0.0722f - 0.0722f * oneMinusAmount,
                               0.2126f - 0.2126f * oneMinusAmount, 0.7152f - 0.7152f * oneMinusAmount,
                               0.0722f + 0.9278f * oneMinusAmount};
  }

  KRYS_NODISCARD constexpr ColourMatrix<5, 4> InvertColourMatrix(float amount) noexcept
  {
    // Invert is specified as a component transfer function:
    // https://www.w3.org/TR/filter-effects-1/#invertEquivalent which is equivalent to the following matrix.
    amount = std::clamp(amount, 0.0f, 1.0f);
    float multiplier = 1.0f - amount * 2.0f;
    return ColourMatrix<5, 4> {multiplier, 0.0f, 0.0f,   0.0f, amount, 0.0f,       multiplier,
                               0.0f,       0.0f, amount, 0.0f, 0.0f,   multiplier, 0.0f,
                               amount,     0.0f, 0.0f,   0.0f, 1.0f,   0.0f};
  }

  KRYS_NODISCARD constexpr ColourMatrix<5, 4> OpacityColourMatrix(float amount) noexcept
  {
    // Opacity is specified as a component transfer function:
    // https://www.w3.org/TR/filter-effects-1/#opacityEquivalent which is equivalent to the following matrix.
    amount = std::clamp(amount, 0.0f, 1.0f);
    return ColourMatrix<5, 4> {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,   0.0f,
                               0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, amount, 0.0f};
  }

  KRYS_NODISCARD constexpr ColourMatrix<3, 3> SepiaColourMatrix(float amount) noexcept
  {
    // Values from https://www.w3.org/TR/filter-effects-1/#sepiaEquivalent
    float oneMinusAmount = std::clamp(1.0f - amount, 0.0f, 1.0f);
    return ColourMatrix<3, 3> {
      0.393f + 0.607f * oneMinusAmount, 0.769f - 0.769f * oneMinusAmount, 0.189f - 0.189f * oneMinusAmount,
      0.349f - 0.349f * oneMinusAmount, 0.686f + 0.314f * oneMinusAmount, 0.168f - 0.168f * oneMinusAmount,
      0.272f - 0.272f * oneMinusAmount, 0.534f - 0.534f * oneMinusAmount, 0.131f + 0.869f * oneMinusAmount};
  }

  KRYS_NODISCARD constexpr ColourMatrix<3, 3> SaturationColourMatrix(float amount) noexcept
  {
    // Values from https://www.w3.org/TR/filter-effects-1/#feColorMatrixElement
    return ColourMatrix<3, 3> {0.213f + 0.787f * amount, 0.715f - 0.715f * amount, 0.072f - 0.072f * amount,
                               0.213f - 0.213f * amount, 0.715f + 0.285f * amount, 0.072f - 0.072f * amount,
                               0.213f - 0.213f * amount, 0.715f - 0.715f * amount, 0.072f + 0.928f * amount};
  }

  // NOTE: HueRotateColourMatrix is not constexpr due to use of cos/sin which are not constexpr yet.
  KRYS_NODISCARD inline ColourMatrix<3, 3> HueRotateColourMatrix(float angleInDegrees) noexcept
  {
    float cosHue = cos(deg2rad(angleInDegrees));
    float sinHue = sin(deg2rad(angleInDegrees));

    // Values from https://www.w3.org/TR/filter-effects-1/#feColorMatrixElement
    return ColourMatrix<3, 3> {
      0.213f + cosHue * 0.787f - sinHue * 0.213f, 0.715f - cosHue * 0.715f - sinHue * 0.715f,
      0.072f - cosHue * 0.072f + sinHue * 0.928f, 0.213f - cosHue * 0.213f + sinHue * 0.143f,
      0.715f + cosHue * 0.285f + sinHue * 0.140f, 0.072f - cosHue * 0.072f - sinHue * 0.283f,
      0.213f - cosHue * 0.213f - sinHue * 0.787f, 0.715f - cosHue * 0.715f + sinHue * 0.715f,
      0.072f + cosHue * 0.928f + sinHue * 0.072f};
  }

  template <size_t ColumnCount, size_t RowCount>
  template <size_t NumberOfComponents>
  KRYS_NODISCARD constexpr auto ColourMatrix<ColumnCount, RowCount>::TransformedColourComponents(
    const ColourComponents<float, NumberOfComponents> &inputVector) const noexcept
    -> ColourComponents<float, NumberOfComponents>
  {
    static_assert(ColourComponents<float, NumberOfComponents>::Size >= RowCount);

    ColourComponents<float, NumberOfComponents> result;
    for (size_t row = 0uz; row < RowCount; ++row)
    {
      if constexpr (ColumnCount <= ColourComponents<float, NumberOfComponents>::Size)
      {
        for (size_t column = 0uz; column < ColumnCount; ++column)
        {
          result[row] += at(row, column) * inputVector[column];
        }
      }
      else if constexpr (ColumnCount > ColourComponents<float, NumberOfComponents>::Size)
      {
        for (size_t column = 0uz; column < ColourComponents<float, NumberOfComponents>::Size; ++column)
        {
          result[row] += at(row, column) * inputVector[column];
        }

        for (size_t additionalColumn = ColourComponents<float, NumberOfComponents>::Size;
             additionalColumn < ColumnCount; ++additionalColumn)
        {
          result[row] += at(row, additionalColumn);
        }
      }
    }
    if constexpr (ColourComponents<float, NumberOfComponents>::Size > RowCount)
    {
      for (size_t additionalRow = RowCount; additionalRow < ColourComponents<float, NumberOfComponents>::Size;
           ++additionalRow)
      {
        result[additionalRow] = inputVector[additionalRow];
      }
    }

    return result;
  }

  template <typename T, typename M>
  KRYS_NODISCARD constexpr auto ApplyMatricesToColourComponents(const ColourComponents<T, 4> &components,
                                                                M matrix) noexcept -> ColourComponents<T, 4>
  {
    return matrix.TransformedColourComponents(components);
  }

  template <typename T, typename M, typename... Matrices>
  KRYS_NODISCARD constexpr auto ApplyMatricesToColourComponents(const ColourComponents<T, 4> &components,
                                                                M matrix, Matrices... matrices) noexcept
    -> ColourComponents<T, 4>
  {
    return ApplyMatricesToColourComponents(matrix.TransformedColourComponents(components), matrices...);
  }
}