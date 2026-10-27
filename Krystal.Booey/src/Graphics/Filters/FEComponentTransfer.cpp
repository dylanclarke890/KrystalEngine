#include "Krystal.Booey/Graphics/Filters/FEComponentTransfer.hpp"
#include "Krystal.Core/Text/TextStream.hpp"
#include <algorithm>
#include <cmath>

namespace krys::boo
{
  Ref<FEComponentTransfer> FEComponentTransfer::Create(const ComponentTransferFunction &redFunction,
                                                       const ComponentTransferFunction &greenFunction,
                                                       const ComponentTransferFunction &blueFunction,
                                                       const ComponentTransferFunction &alphaFunction,
                                                       const DestinationColorSpace &colorSpace) noexcept
  {
    return AdoptRef(
      *new FEComponentTransfer(redFunction, greenFunction, blueFunction, alphaFunction, colorSpace));
  }

  Ref<FEComponentTransfer> FEComponentTransfer::Create(ComponentTransferFunctions &&functions) noexcept
  {
    return AdoptRef(*new FEComponentTransfer(krys::move(functions)));
  }

  FEComponentTransfer::FEComponentTransfer(const ComponentTransferFunction &redFunction,
                                           const ComponentTransferFunction &greenFunction,
                                           const ComponentTransferFunction &blueFunction,
                                           const ComponentTransferFunction &alphaFunction,
                                           const DestinationColorSpace &colorSpace) noexcept
      : FilterEffect(FilterEffect::Type::FEComponentTransfer, colorSpace),
        _functions {redFunction, greenFunction, blueFunction, alphaFunction}
  {
  }

  FEComponentTransfer::FEComponentTransfer(ComponentTransferFunctions &&functions) noexcept
      : FilterEffect(FilterEffect::Type::FEComponentTransfer), _functions(krys::move(functions))
  {
  }

  bool FEComponentTransfer::operator==(const FEComponentTransfer &other) const noexcept
  {
    return FilterEffect::operator==(other) && _functions == other._functions;
  }

  bool FEComponentTransfer::SetType(ComponentTransferChannel channel, ComponentTransferType type) noexcept
  {
    if (FunctionOf(channel).Type == type)
    {
      return false;
    }
    FunctionOf(channel).Type = type;
    return true;
  }

  bool FEComponentTransfer::SetSlope(ComponentTransferChannel channel, float value) noexcept
  {
    if (FunctionOf(channel).Slope == value)
    {
      return false;
    }
    FunctionOf(channel).Slope = value;
    return true;
  }

  bool FEComponentTransfer::SetIntercept(ComponentTransferChannel channel, float value) noexcept
  {
    if (FunctionOf(channel).Intercept == value)
    {
      return false;
    }
    FunctionOf(channel).Intercept = value;
    return true;
  }

  bool FEComponentTransfer::SetAmplitude(ComponentTransferChannel channel, float value) noexcept
  {
    if (FunctionOf(channel).Amplitude == value)
    {
      return false;
    }
    FunctionOf(channel).Amplitude = value;
    return true;
  }

  bool FEComponentTransfer::SetExponent(ComponentTransferChannel channel, float value) noexcept
  {
    if (FunctionOf(channel).Exponent == value)
    {
      return false;
    }
    FunctionOf(channel).Exponent = value;
    return true;
  }

  bool FEComponentTransfer::SetOffset(ComponentTransferChannel channel, float value) noexcept
  {
    if (FunctionOf(channel).Offset == value)
    {
      return false;
    }
    FunctionOf(channel).Offset = value;
    return true;
  }

  bool FEComponentTransfer::SetTableValues(ComponentTransferChannel channel, List<float> &&values) noexcept
  {
    if (FunctionOf(channel).TableValues == values)
    {
      return false;
    }
    FunctionOf(channel).TableValues = krys::move(values);
    return true;
  }

  namespace
  {
    TextStream &operator<<(TextStream &ts, ComponentTransferType type) noexcept
    {
      switch (type)
      {
        case ComponentTransferType::FECOMPONENTTRANSFER_TYPE_UNKNOWN:  ts << u8"UNKNOWN"; break;
        case ComponentTransferType::FECOMPONENTTRANSFER_TYPE_IDENTITY: ts << u8"IDENTITY"; break;
        case ComponentTransferType::FECOMPONENTTRANSFER_TYPE_TABLE:    ts << u8"TABLE"; break;
        case ComponentTransferType::FECOMPONENTTRANSFER_TYPE_DISCRETE: ts << u8"DISCRETE"; break;
        case ComponentTransferType::FECOMPONENTTRANSFER_TYPE_LINEAR:   ts << u8"LINEAR"; break;
        case ComponentTransferType::FECOMPONENTTRANSFER_TYPE_GAMMA:    ts << u8"GAMMA"; break;
      }
      return ts;
    }

    TextStream &operator<<(TextStream &ts, const ComponentTransferFunction &function) noexcept
    {
      ts << u8"type=\"" << function.Type;

      switch (function.Type)
      {
        case ComponentTransferType::FECOMPONENTTRANSFER_TYPE_UNKNOWN:  break;
        case ComponentTransferType::FECOMPONENTTRANSFER_TYPE_IDENTITY: break;
        case ComponentTransferType::FECOMPONENTTRANSFER_TYPE_TABLE:
        case ComponentTransferType::FECOMPONENTTRANSFER_TYPE_DISCRETE:
          ts << u8' ' << function.TableValues;
          break;
        case ComponentTransferType::FECOMPONENTTRANSFER_TYPE_LINEAR:
          ts << u8"\" slope=\"" << function.Slope << u8"\" intercept=\"" << function.Intercept << u8'"';
          break;
        case ComponentTransferType::FECOMPONENTTRANSFER_TYPE_GAMMA:
          ts << u8"\" amplitude=\"" << function.Amplitude << u8"\" exponent=\"" << function.Exponent
             << u8"\" offset=\"" << function.Offset << u8'"';
          break;
      }

      return ts;
    }
  }

  TextStream &FEComponentTransfer::ExternalRepresentation(TextStream &ts,
                                                          FilterRepresentation representation) const noexcept
  {
    ts << Indent << u8"[feComponentTransfer";
    FilterEffect::ExternalRepresentation(ts, representation);
    ts << u8'\n';
    {
      TextStream::IndentScope indentScope(ts, 2);
      ts << Indent << u8"{red: " << FunctionOf(ComponentTransferChannel::Red) << u8"}\n";
      ts << Indent << u8"{green: " << FunctionOf(ComponentTransferChannel::Green) << u8"}\n";
      ts << Indent << u8"{blue: " << FunctionOf(ComponentTransferChannel::Blue) << u8"}\n";
      ts << Indent << u8"{alpha: " << FunctionOf(ComponentTransferChannel::Alpha) << u8'}';
    }
    ts << u8"]\n";
    return ts;
  }

  FEComponentTransfer::LookupTable
    FEComponentTransfer::ComputeLookupTable(const ComponentTransferFunction &function) noexcept
  {
    LookupTable values;
    for (size_t i = 0; i < values.size(); ++i)
    {
      values[i] = static_cast<uint8>(i);
    }

    switch (function.Type)
    {
      case ComponentTransferType::FECOMPONENTTRANSFER_TYPE_UNKNOWN:
      case ComponentTransferType::FECOMPONENTTRANSFER_TYPE_IDENTITY: break;

      case ComponentTransferType::FECOMPONENTTRANSFER_TYPE_TABLE:
      {
        const auto &tableValues = function.TableValues;
        auto n = static_cast<uint32>(tableValues.size());
        if (n < 1)
        {
          break;
        }
        for (uint32 i = 0; i < values.size(); ++i)
        {
          double c = i / 255.0;
          auto k = static_cast<uint32>(c * (n - 1));
          double v1 = tableValues[k];
          double v2 = tableValues[std::min((k + 1), (n - 1))];
          double val = 255.0 * (v1 + (c * (n - 1) - k) * (v2 - v1));
          val = std::max(0.0, std::min(255.0, val));
          values[i] = static_cast<uint8>(val);
        }
        break;
      }

      case ComponentTransferType::FECOMPONENTTRANSFER_TYPE_DISCRETE:
      {
        const auto &tableValues = function.TableValues;
        auto n = static_cast<uint32>(tableValues.size());
        if (n < 1)
        {
          break;
        }
        for (uint32 i = 0; i < values.size(); ++i)
        {
          auto k = static_cast<uint32>((i * n) / 255.0);
          k = std::min(k, n - 1);
          double val = 255 * tableValues[k];
          val = std::max(0.0, std::min(255.0, val));
          values[i] = static_cast<uint8>(val);
        }
        break;
      }

      case ComponentTransferType::FECOMPONENTTRANSFER_TYPE_LINEAR:
      {
        for (uint32 i = 0; i < values.size(); ++i)
        {
          double val = function.Slope * i + 255 * function.Intercept;
          val = std::max(0.0, std::min(255.0, val));
          values[i] = static_cast<uint8>(val);
        }
        break;
      }

      case ComponentTransferType::FECOMPONENTTRANSFER_TYPE_GAMMA:
      {
        for (uint32 i = 0; i < values.size(); ++i)
        {
          double exponent = function.Exponent;
          double val = 255.0 * (function.Amplitude * std::pow((i / 255.0), exponent) + function.Offset);
          val = std::max(0.0, std::min(255.0, val));
          values[i] = static_cast<uint8>(val);
        }
        break;
      }
    }

    return values;
  }
}
