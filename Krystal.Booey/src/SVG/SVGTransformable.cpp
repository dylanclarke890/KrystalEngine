#include "Krystal.Booey/SVG/SVGTransformable.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Booey/SVG/Internals/SVGParserUtilities.hpp"
#include "Krystal.Booey/SVG/SVGTransform.hpp"
#include "Krystal.Core/Text/ParsingUtilities.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/Span.hpp"

namespace krys::boo::svg
{
  namespace
  {
    int32 ParseTransformParamList(dom::DOMStringView &buffer, Span<float, 6> values, int32 required,
                                  int32 optional) noexcept
    {
      int32 optionalParams = 0, requiredParams = 0;

      if (!SkipOptionalSVGSpaces(buffer) || buffer.front() != u8'(')
      {
        return -1;
      }

      buffer.remove_prefix(1);

      SkipOptionalSVGSpaces(buffer);

      while (requiredParams < required)
      {
        if (buffer.empty())
        {
          return -1;
        }
        auto parsedNumber = ParseNumber(buffer, SuffixSkippingPolicy::DontSkip);
        if (!parsedNumber)
        {
          return -1;
        }
        values[requiredParams] = *parsedNumber;
        requiredParams++;
        if (requiredParams < required)
        {
          SkipOptionalSVGSpacesOrDelimiter(buffer);
        }
      }
      if (!SkipOptionalSVGSpaces(buffer))
      {
        return -1;
      }

      bool delimParsed = SkipOptionalSVGSpacesOrDelimiter(buffer);

      if (buffer.empty())
      {
        return -1;
      }

      if (buffer.front() == u8')')
      {
        // skip optionals
        buffer.remove_prefix(1);
        if (delimParsed)
        {
          return -1;
        }
      }
      else
      {
        while (optionalParams < optional)
        {
          if (buffer.empty())
          {
            return -1;
          }
          auto parsedNumber = ParseNumber(buffer, SuffixSkippingPolicy::DontSkip);
          if (!parsedNumber)
          {
            return -1;
          }
          values[requiredParams + optionalParams] = *parsedNumber;
          optionalParams++;
          if (optionalParams < optional)
          {
            SkipOptionalSVGSpacesOrDelimiter(buffer);
          }
        }

        if (!SkipOptionalSVGSpaces(buffer))
        {
          return -1;
        }

        delimParsed = SkipOptionalSVGSpacesOrDelimiter(buffer);

        if (buffer.empty() || buffer.front() != u8')' || delimParsed)
        {
          return -1;
        }
        buffer.remove_prefix(1);
      }

      return requiredParams + optionalParams;
    }

    // These should be kept in sync with enum SVGTransformType
    constexpr Array<int32, 7> RequiredValuesForType {0, 6, 1, 1, 1, 1, 1};
    constexpr Array<int32, 7> OptionalValuesForType {0, 0, 1, 1, 2, 0, 0};
  }

  SVGTransformable::~SVGTransformable() noexcept = default;

  bool SVGTransformable::ParseAndReplaceTransform(SVGTransformValue::SVGTransformType type,
                                                  dom::DOMStringView &buffer,
                                                  SVGTransform &transform) noexcept
  {
    krys_assert(type == transform.Value().Type());
    if (type == SVGTransformValue::SVG_TRANSFORM_UNKNOWN)
    {
      return false;
    }

    Array<float, 6> values {0, 0, 0, 0, 0, 0};
    int32 valueCount =
      ParseTransformParamList(buffer, values, RequiredValuesForType[type], OptionalValuesForType[type]);
    if (valueCount < 0)
    {
      return false;
    }

    switch (type)
    {
      case SVGTransformValue::SVG_TRANSFORM_UNKNOWN: krys_assert_msg(false, "Not reached"); return false;
      case SVGTransformValue::SVG_TRANSFORM_SKEWX:   transform.Value().SetSkewX(values[0]); return true;
      case SVGTransformValue::SVG_TRANSFORM_SKEWY:   transform.Value().SetSkewY(values[0]); return true;
      case SVGTransformValue::SVG_TRANSFORM_SCALE:
        if (valueCount == 1)
        {
          transform.Value().SetScale(values[0], values[0]);
        }
        else
        {
          transform.Value().SetScale(values[0], values[1]);
        }
        return true;
      case SVGTransformValue::SVG_TRANSFORM_TRANSLATE:
        if (valueCount == 1)
        {
          transform.Value().SetTranslate(values[0], 0);
        }
        else
        {
          transform.Value().SetTranslate(values[0], values[1]);
        }
        return true;
      case SVGTransformValue::SVG_TRANSFORM_ROTATE:
        if (valueCount == 1)
        {
          transform.Value().SetRotate(values[0], 0, 0);
        }
        else
        {
          transform.Value().SetRotate(values[0], values[1], values[2]);
        }
        return true;
      case SVGTransformValue::SVG_TRANSFORM_MATRIX:
        transform.Value().SetMatrix(
          AffineTransform(values[0], values[1], values[2], values[3], values[4], values[5]));
        return true;
    }

    return false;
  }

  RefPtr<SVGTransform> SVGTransformable::ParseTransform(SVGTransformValue::SVGTransformType type,
                                                        dom::DOMStringView &buffer) noexcept
  {
    if (type == SVGTransformValue::SVG_TRANSFORM_UNKNOWN)
    {
      return nullptr;
    }

    Array<float, 6> values {0, 0, 0, 0, 0, 0};
    int32 valueCount =
      ParseTransformParamList(buffer, values, RequiredValuesForType[type], OptionalValuesForType[type]);
    if (valueCount < 0)
    {
      return nullptr;
    }

    switch (type)
    {
      case SVGTransformValue::SVG_TRANSFORM_UNKNOWN: krys_assert_msg(false, "Not reached"); return nullptr;
      case SVGTransformValue::SVG_TRANSFORM_SKEWX:
      {
        SVGTransformValue transform;
        transform.SetSkewX(values[0]);
        return SVGTransform::Create(krys::move(transform));
      }
      case SVGTransformValue::SVG_TRANSFORM_SKEWY:
      {
        SVGTransformValue transform;
        transform.SetSkewY(values[0]);
        return SVGTransform::Create(krys::move(transform));
      }
      case SVGTransformValue::SVG_TRANSFORM_SCALE:
      {
        auto resultValue = [&]()
        {
          // Spec: if only one param given, assume uniform scaling
          if (valueCount == 1)
          {
            return SVGTransformValue::ScaleTransformValue({values[0], values[0]});
          }
          return SVGTransformValue::ScaleTransformValue({values[0], values[1]});
        };
        return SVGTransform::Create(resultValue());
      }
      case SVGTransformValue::SVG_TRANSFORM_TRANSLATE:
      {
        // Spec: if only one param given, assume 2nd param to be 0
        if (valueCount == 1)
        {
          return SVGTransform::Create(SVGTransformValue::TranslateTransformValue({values[0], 0}));
        }
        return SVGTransform::Create(SVGTransformValue::TranslateTransformValue({values[0], values[1]}));
      }
      case SVGTransformValue::SVG_TRANSFORM_ROTATE:
      {
        auto resultValue = [&]()
        {
          if (valueCount == 1)
          {
            return SVGTransformValue::RotateTransformValue(values[0], {});
          }
          return SVGTransformValue::RotateTransformValue(values[0], {values[1], values[2]});
        };
        return SVGTransform::Create(resultValue());
      }
      case SVGTransformValue::SVG_TRANSFORM_MATRIX:
      {
        SVGTransformValue transform;
        transform.SetMatrix(
          AffineTransform(values[0], values[1], values[2], values[3], values[4], values[5]));
        return SVGTransform::Create(transform);
      }
    }

    return nullptr;
  }

  Maybe<SVGTransformValue::SVGTransformType>
    SVGTransformable::ParseTransformTypeFromString(dom::DOMStringView string) noexcept
  {
    return ParseTransformType(string);
  }

  Maybe<SVGTransformValue::SVGTransformType>
    SVGTransformable::ParseTransformType(dom::DOMStringView &buffer) noexcept
  {
    if (buffer.empty())
    {
      return {};
    }

    if (buffer.front() == u8's')
    {
      if (text::SkipCharactersExactly(buffer, u8"skewX"))
      {
        return SVGTransformValue::SVG_TRANSFORM_SKEWX;
      }
      if (text::SkipCharactersExactly(buffer, u8"skewY"))
      {
        return SVGTransformValue::SVG_TRANSFORM_SKEWY;
      }
      if (text::SkipCharactersExactly(buffer, u8"scale"))
      {
        return SVGTransformValue::SVG_TRANSFORM_SCALE;
      }
      return {};
    }
    if (text::SkipCharactersExactly(buffer, u8"translate"))
    {
      return SVGTransformValue::SVG_TRANSFORM_TRANSLATE;
    }
    if (text::SkipCharactersExactly(buffer, u8"rotate"))
    {
      return SVGTransformValue::SVG_TRANSFORM_ROTATE;
    }
    if (text::SkipCharactersExactly(buffer, u8"matrix"))
    {
      return SVGTransformValue::SVG_TRANSFORM_MATRIX;
    }
    return {};
  }
}
