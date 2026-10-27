#include "Krystal.Booey/Graphics/Filters/FEColorMatrix.hpp"
#include "Krystal.Booey/Graphics/Filters/Filter.hpp"
#include "Krystal.Core/Colour/ColourMatrix.hpp"
#include "Krystal.Core/Text/TextStream.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

namespace krys::boo
{
  namespace
  {
    /// @brief The value as a finite float: infinities become the largest finite floats.
    KRYS_NODISCARD float NormalizedFloat(float value) noexcept
    {
      constexpr float max = std::numeric_limits<float>::max();
      if (value > max)
      {
        return max;
      }
      if (value < -max)
      {
        return -max;
      }
      return value;
    }
  }

  Ref<FEColorMatrix> FEColorMatrix::Create(ColorMatrixType type, List<float> &&values) noexcept
  {
    return AdoptRef(*new FEColorMatrix(type, krys::move(values)));
  }

  FEColorMatrix::FEColorMatrix(ColorMatrixType type, List<float> &&values) noexcept
      : FilterEffect(FilterEffect::Type::FEColorMatrix), _type(type), _values(krys::move(values))
  {
  }

  bool FEColorMatrix::operator==(const FEColorMatrix &other) const noexcept
  {
    return FilterEffect::operator==(other) && _type == other._type && _values == other._values;
  }

  bool FEColorMatrix::SetType(ColorMatrixType type) noexcept
  {
    if (_type == type)
    {
      return false;
    }
    _type = type;
    return true;
  }

  bool FEColorMatrix::SetValues(const List<float> &values) noexcept
  {
    if (_values == values)
    {
      return false;
    }
    _values = values;
    return true;
  }

  void FEColorMatrix::CalculateSaturateComponents(Span<float, 9> components, float value) noexcept
  {
    auto saturationMatrix = SaturationColourMatrix(value);

    components[0] = saturationMatrix.at(0, 0);
    components[1] = saturationMatrix.at(0, 1);
    components[2] = saturationMatrix.at(0, 2);

    components[3] = saturationMatrix.at(1, 0);
    components[4] = saturationMatrix.at(1, 1);
    components[5] = saturationMatrix.at(1, 2);

    components[6] = saturationMatrix.at(2, 0);
    components[7] = saturationMatrix.at(2, 1);
    components[8] = saturationMatrix.at(2, 2);
  }

  void FEColorMatrix::CalculateHueRotateComponents(Span<float, 9> components, float angleInDegrees) noexcept
  {
    auto hueRotateMatrix = HueRotateColourMatrix(angleInDegrees);

    components[0] = hueRotateMatrix.at(0, 0);
    components[1] = hueRotateMatrix.at(0, 1);
    components[2] = hueRotateMatrix.at(0, 2);

    components[3] = hueRotateMatrix.at(1, 0);
    components[4] = hueRotateMatrix.at(1, 1);
    components[5] = hueRotateMatrix.at(1, 2);

    components[6] = hueRotateMatrix.at(2, 0);
    components[7] = hueRotateMatrix.at(2, 1);
    components[8] = hueRotateMatrix.at(2, 2);
  }

  List<float> FEColorMatrix::NormalizedFloats(const List<float> &values) noexcept
  {
    List<float> normalizedValues(values.size());
    for (size_t i = 0; i < values.size(); ++i)
    {
      normalizedValues[i] = NormalizedFloat(values[i]);
    }
    return normalizedValues;
  }

  bool FEColorMatrix::ResultIsAlphaImage(Span<const Ref<FilterImage>>) const noexcept
  {
    return _type == ColorMatrixType::FECOLORMATRIX_TYPE_LUMINANCETOALPHA;
  }

  EnumSet<FilterRenderingMode> FEColorMatrix::SupportedFilterRenderingModes(
    EnumSet<FilterRenderingMode> preferredFilterRenderingModes) const noexcept
  {
    // The accelerated backend applies every type. The graphics context applies a matrix, a saturation or a
    // hue rotation; luminance to alpha needs an image.
    EnumSet<FilterRenderingMode> modes = FilterRenderingMode::Accelerated;
    if (_type == ColorMatrixType::FECOLORMATRIX_TYPE_MATRIX
        || _type == ColorMatrixType::FECOLORMATRIX_TYPE_SATURATE
        || _type == ColorMatrixType::FECOLORMATRIX_TYPE_HUEROTATE)
    {
      modes.Add(FilterRenderingMode::GraphicsContext);
    }
    return modes & preferredFilterRenderingModes;
  }

  Maybe<GraphicsStyle> FEColorMatrix::CreateGraphicsStyle(GraphicsContext &, const Filter &) const noexcept
  {
    switch (_type)
    {
      case ColorMatrixType::FECOLORMATRIX_TYPE_MATRIX:
      {
        krys_assert(_values.size() == 20u);
        GraphicsColorMatrix result {};
        std::copy_n(_values.begin(), std::min<size_t>(_values.size(), 20u), result.Values.begin());
        return result;
      }
      case ColorMatrixType::FECOLORMATRIX_TYPE_SATURATE:
        return GraphicsColorMatrix {ColourMatrix<5, 4>(SaturationColourMatrix(_values[0])).data()};

      case ColorMatrixType::FECOLORMATRIX_TYPE_HUEROTATE:
        return GraphicsColorMatrix {ColourMatrix<5, 4>(HueRotateColourMatrix(_values[0])).data()};

      default: krys_debug_assert(false); break;
    }

    return null;
  }

  namespace
  {
    TextStream &operator<<(TextStream &ts, const ColorMatrixType &type) noexcept
    {
      switch (type)
      {
        case ColorMatrixType::FECOLORMATRIX_TYPE_UNKNOWN:          ts << u8"UNKNOWN"; break;
        case ColorMatrixType::FECOLORMATRIX_TYPE_MATRIX:           ts << u8"MATRIX"; break;
        case ColorMatrixType::FECOLORMATRIX_TYPE_SATURATE:         ts << u8"SATURATE"; break;
        case ColorMatrixType::FECOLORMATRIX_TYPE_HUEROTATE:        ts << u8"HUEROTATE"; break;
        case ColorMatrixType::FECOLORMATRIX_TYPE_LUMINANCETOALPHA: ts << u8"LUMINANCETOALPHA"; break;
      }
      return ts;
    }
  }

  TextStream &FEColorMatrix::ExternalRepresentation(TextStream &ts,
                                                    FilterRepresentation representation) const noexcept
  {
    ts << Indent << u8"[feColorMatrix";
    FilterEffect::ExternalRepresentation(ts, representation);

    ts << u8" type=\"" << _type << u8'"';
    if (!_values.empty())
    {
      ts << u8" values=\"";
      bool isFirst = true;
      for (auto value : _values)
      {
        if (isFirst)
        {
          isFirst = false;
        }
        else
        {
          ts << u8' ';
        }
        ts << value;
      }
      ts << u8'"';
    }

    ts << u8"]\n";
    return ts;
  }
}
