#include "Krystal.Booey/SVG/SVGPreserveAspectRatioValue.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Booey/SVG/Internals/SVGParserUtilities.hpp"
#include "Krystal.Core/Text/ParsingUtilities.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::svg
{
  SVGPreserveAspectRatioValue::SVGPreserveAspectRatioValue(dom::DOMStringView value) noexcept
  {
    Parse(value);
  }

  SVGPreserveAspectRatioValue::SVGPreserveAspectRatioValue(SVGPreserveAspectRatioType align,
                                                           SVGMeetOrSliceType meetOrSlice) noexcept
      : _align(align), _meetOrSlice(meetOrSlice)
  {
  }

  dom::ExceptionOr<void> SVGPreserveAspectRatioValue::SetAlign(uint16 align) noexcept
  {
    if (align == SVG_PRESERVEASPECTRATIO_UNKNOWN || align > SVG_PRESERVEASPECTRATIO_XMAXYMAX)
    {
      return dom::Exception {dom::ExceptionCode::NotSupportedError};
    }

    _align = static_cast<SVGPreserveAspectRatioType>(align);
    return {};
  }

  dom::ExceptionOr<void> SVGPreserveAspectRatioValue::SetMeetOrSlice(uint16 meetOrSlice) noexcept
  {
    if (meetOrSlice == SVG_MEETORSLICE_UNKNOWN || meetOrSlice > SVG_MEETORSLICE_SLICE)
    {
      return dom::Exception {dom::ExceptionCode::NotSupportedError};
    }

    _meetOrSlice = static_cast<SVGMeetOrSliceType>(meetOrSlice);
    return {};
  }

  bool SVGPreserveAspectRatioValue::Parse(dom::DOMStringView value) noexcept
  {
    return ParseInternal(value, true);
  }

  bool SVGPreserveAspectRatioValue::Parse(dom::DOMStringView &buffer, bool validate) noexcept
  {
    return ParseInternal(buffer, validate);
  }

  bool SVGPreserveAspectRatioValue::ParseInternal(dom::DOMStringView &buffer, bool validate) noexcept
  {
    SVGPreserveAspectRatioType align = SVG_PRESERVEASPECTRATIO_XMIDYMID;
    SVGMeetOrSliceType meetOrSlice = SVG_MEETORSLICE_MEET;

    _align = align;
    _meetOrSlice = meetOrSlice;

    if (!SkipOptionalSVGSpaces(buffer))
    {
      return false;
    }

    if (buffer.front() == u8'n')
    {
      if (!text::SkipCharactersExactly(buffer, u8"none"))
      {
        return false;
      }
      align = SVG_PRESERVEASPECTRATIO_NONE;
      SkipOptionalSVGSpaces(buffer);
    }
    else if (buffer.front() == u8'x')
    {
      if (buffer.size() < 8)
      {
        return false;
      }
      if (buffer[1] != u8'M' || buffer[4] != u8'Y' || buffer[5] != u8'M')
      {
        return false;
      }
      if (buffer[2] == u8'i')
      {
        if (buffer[3] == u8'n')
        {
          if (buffer[6] == u8'i')
          {
            if (buffer[7] == u8'n')
            {
              align = SVG_PRESERVEASPECTRATIO_XMINYMIN;
            }
            else if (buffer[7] == u8'd')
            {
              align = SVG_PRESERVEASPECTRATIO_XMINYMID;
            }
            else
            {
              return false;
            }
          }
          else if (buffer[6] == u8'a' && buffer[7] == u8'x')
          {
            align = SVG_PRESERVEASPECTRATIO_XMINYMAX;
          }
          else
          {
            return false;
          }
        }
        else if (buffer[3] == u8'd')
        {
          if (buffer[6] == u8'i')
          {
            if (buffer[7] == u8'n')
            {
              align = SVG_PRESERVEASPECTRATIO_XMIDYMIN;
            }
            else if (buffer[7] == u8'd')
            {
              align = SVG_PRESERVEASPECTRATIO_XMIDYMID;
            }
            else
            {
              return false;
            }
          }
          else if (buffer[6] == u8'a' && buffer[7] == u8'x')
          {
            align = SVG_PRESERVEASPECTRATIO_XMIDYMAX;
          }
          else
          {
            return false;
          }
        }
        else
        {
          return false;
        }
      }
      else if (buffer[2] == u8'a' && buffer[3] == u8'x')
      {
        if (buffer[6] == u8'i')
        {
          if (buffer[7] == u8'n')
          {
            align = SVG_PRESERVEASPECTRATIO_XMAXYMIN;
          }
          else if (buffer[7] == u8'd')
          {
            align = SVG_PRESERVEASPECTRATIO_XMAXYMID;
          }
          else
          {
            return false;
          }
        }
        else if (buffer[6] == u8'a' && buffer[7] == u8'x')
        {
          align = SVG_PRESERVEASPECTRATIO_XMAXYMAX;
        }
        else
        {
          return false;
        }
      }
      else
      {
        return false;
      }
      buffer.remove_prefix(8);
      SkipOptionalSVGSpaces(buffer);
    }
    else
    {
      return false;
    }

    if (!buffer.empty())
    {
      if (buffer.front() == u8'm')
      {
        if (!text::SkipCharactersExactly(buffer, u8"meet"))
        {
          return false;
        }
        SkipOptionalSVGSpaces(buffer);
      }
      else if (buffer.front() == u8's')
      {
        if (!text::SkipCharactersExactly(buffer, u8"slice"))
        {
          return false;
        }
        SkipOptionalSVGSpaces(buffer);
        if (align != SVG_PRESERVEASPECTRATIO_NONE)
        {
          meetOrSlice = SVG_MEETORSLICE_SLICE;
        }
      }
    }

    if (!buffer.empty() && validate)
    {
      return false;
    }

    _align = align;
    _meetOrSlice = meetOrSlice;

    return true;
  }

  void SVGPreserveAspectRatioValue::TransformRect(FloatRect &destRect, FloatRect &srcRect) const noexcept
  {
    if (_align == SVG_PRESERVEASPECTRATIO_NONE)
    {
      return;
    }

    FloatSize imageSize = srcRect.Size();
    float origDestWidth = destRect.Width();
    float origDestHeight = destRect.Height();
    switch (_meetOrSlice)
    {
      case SVGPreserveAspectRatioValue::SVG_MEETORSLICE_UNKNOWN:
      {
        break;
      }
      case SVGPreserveAspectRatioValue::SVG_MEETORSLICE_MEET:
      {
        float widthToHeightMultiplier = srcRect.Height() / srcRect.Width();
        if (origDestHeight > origDestWidth * widthToHeightMultiplier)
        {
          destRect.SetHeight(origDestWidth * widthToHeightMultiplier);
          switch (_align)
          {
            case SVGPreserveAspectRatioValue::SVG_PRESERVEASPECTRATIO_XMINYMID:
            case SVGPreserveAspectRatioValue::SVG_PRESERVEASPECTRATIO_XMIDYMID:
            case SVGPreserveAspectRatioValue::SVG_PRESERVEASPECTRATIO_XMAXYMID:
            {
              destRect.SetY(destRect.Y() + origDestHeight / 2 - destRect.Height() / 2);
              break;
            }
            case SVGPreserveAspectRatioValue::SVG_PRESERVEASPECTRATIO_XMINYMAX:
            case SVGPreserveAspectRatioValue::SVG_PRESERVEASPECTRATIO_XMIDYMAX:
            case SVGPreserveAspectRatioValue::SVG_PRESERVEASPECTRATIO_XMAXYMAX:
            {
              destRect.SetY(destRect.Y() + origDestHeight - destRect.Height());
              break;
            }
            default:
            {
              break;
            }
          }
        }
        if (origDestWidth > origDestHeight / widthToHeightMultiplier)
        {
          destRect.SetWidth(origDestHeight / widthToHeightMultiplier);
          switch (_align)
          {
            case SVGPreserveAspectRatioValue::SVG_PRESERVEASPECTRATIO_XMIDYMIN:
            case SVGPreserveAspectRatioValue::SVG_PRESERVEASPECTRATIO_XMIDYMID:
            case SVGPreserveAspectRatioValue::SVG_PRESERVEASPECTRATIO_XMIDYMAX:
            {
              destRect.SetX(destRect.X() + origDestWidth / 2 - destRect.Width() / 2);
              break;
            }
            case SVGPreserveAspectRatioValue::SVG_PRESERVEASPECTRATIO_XMAXYMIN:
            case SVGPreserveAspectRatioValue::SVG_PRESERVEASPECTRATIO_XMAXYMID:
            case SVGPreserveAspectRatioValue::SVG_PRESERVEASPECTRATIO_XMAXYMAX:
            {
              destRect.SetX(destRect.X() + origDestWidth - destRect.Width());
              break;
            }
            default:
            {
              break;
            }
          }
        }
        break;
      }
      case SVGPreserveAspectRatioValue::SVG_MEETORSLICE_SLICE:
      {
        float widthToHeightMultiplier = srcRect.Height() / srcRect.Width();
        // if the destination height is less than the height of the image we'll be drawing
        if (origDestHeight < origDestWidth * widthToHeightMultiplier)
        {
          float destToSrcMultiplier = srcRect.Width() / destRect.Width();
          srcRect.SetHeight(destRect.Height() * destToSrcMultiplier);
          switch (_align)
          {
            case SVGPreserveAspectRatioValue::SVG_PRESERVEASPECTRATIO_XMINYMID:
            case SVGPreserveAspectRatioValue::SVG_PRESERVEASPECTRATIO_XMIDYMID:
            case SVGPreserveAspectRatioValue::SVG_PRESERVEASPECTRATIO_XMAXYMID:
            {
              srcRect.SetY(srcRect.Y() + imageSize.Height() / 2 - srcRect.Height() / 2);
              break;
            }
            case SVGPreserveAspectRatioValue::SVG_PRESERVEASPECTRATIO_XMINYMAX:
            case SVGPreserveAspectRatioValue::SVG_PRESERVEASPECTRATIO_XMIDYMAX:
            case SVGPreserveAspectRatioValue::SVG_PRESERVEASPECTRATIO_XMAXYMAX:
            {
              srcRect.SetY(srcRect.Y() + imageSize.Height() - srcRect.Height());
              break;
            }
            default:
            {
              break;
            }
          }
        }
        // if the destination width is less than the width of the image we'll be drawing
        if (origDestWidth < origDestHeight / widthToHeightMultiplier)
        {
          float destToSrcMultiplier = srcRect.Height() / destRect.Height();
          srcRect.SetWidth(destRect.Width() * destToSrcMultiplier);
          switch (_align)
          {
            case SVGPreserveAspectRatioValue::SVG_PRESERVEASPECTRATIO_XMIDYMIN:
            case SVGPreserveAspectRatioValue::SVG_PRESERVEASPECTRATIO_XMIDYMID:
            case SVGPreserveAspectRatioValue::SVG_PRESERVEASPECTRATIO_XMIDYMAX:
            {
              srcRect.SetX(srcRect.X() + imageSize.Width() / 2 - srcRect.Width() / 2);
              break;
            }
            case SVGPreserveAspectRatioValue::SVG_PRESERVEASPECTRATIO_XMAXYMIN:
            case SVGPreserveAspectRatioValue::SVG_PRESERVEASPECTRATIO_XMAXYMID:
            case SVGPreserveAspectRatioValue::SVG_PRESERVEASPECTRATIO_XMAXYMAX:
            {
              srcRect.SetX(srcRect.X() + imageSize.Width() - srcRect.Width());
              break;
            }
            default:
            {
              break;
            }
          }
        }
        break;
      }
    }
  }

  AffineTransform SVGPreserveAspectRatioValue::GetCTM(float logicalX, float logicalY, float logicalWidth,
                                                      float logicalHeight, float physicalWidth,
                                                      float physicalHeight) const noexcept
  {
    AffineTransform transform;
    if (!logicalWidth || !logicalHeight || !physicalWidth || !physicalHeight)
    {
      krys_assert_msg(false, "Not reached");
      return transform;
    }

    if (_align == SVG_PRESERVEASPECTRATIO_UNKNOWN)
    {
      return transform;
    }

    double extendedLogicalX = logicalX;
    double extendedLogicalY = logicalY;
    double extendedLogicalWidth = logicalWidth;
    double extendedLogicalHeight = logicalHeight;
    double extendedPhysicalWidth = physicalWidth;
    double extendedPhysicalHeight = physicalHeight;
    double logicalRatio = extendedLogicalWidth / extendedLogicalHeight;
    double physicalRatio = extendedPhysicalWidth / extendedPhysicalHeight;

    if (_align == SVG_PRESERVEASPECTRATIO_NONE)
    {
      transform.ScaleNonUniform(extendedPhysicalWidth / extendedLogicalWidth,
                                extendedPhysicalHeight / extendedLogicalHeight);
      transform.Translate(-extendedLogicalX, -extendedLogicalY);
      return transform;
    }

    if ((logicalRatio < physicalRatio && (_meetOrSlice == SVG_MEETORSLICE_MEET))
        || (logicalRatio >= physicalRatio && (_meetOrSlice == SVG_MEETORSLICE_SLICE)))
    {
      transform.ScaleNonUniform(extendedPhysicalHeight / extendedLogicalHeight,
                                extendedPhysicalHeight / extendedLogicalHeight);

      if (_align == SVG_PRESERVEASPECTRATIO_XMINYMIN || _align == SVG_PRESERVEASPECTRATIO_XMINYMID
          || _align == SVG_PRESERVEASPECTRATIO_XMINYMAX)
      {
        transform.Translate(-extendedLogicalX, -extendedLogicalY);
      }
      else if (_align == SVG_PRESERVEASPECTRATIO_XMIDYMIN || _align == SVG_PRESERVEASPECTRATIO_XMIDYMID
               || _align == SVG_PRESERVEASPECTRATIO_XMIDYMAX)
      {
        transform.Translate(
          -extendedLogicalX
            - (extendedLogicalWidth - extendedPhysicalWidth * extendedLogicalHeight / extendedPhysicalHeight)
                / 2,
          -extendedLogicalY);
      }
      else
      {
        transform.Translate(
          -extendedLogicalX
            - (extendedLogicalWidth - extendedPhysicalWidth * extendedLogicalHeight / extendedPhysicalHeight),
          -extendedLogicalY);
      }

      return transform;
    }

    transform.ScaleNonUniform(extendedPhysicalWidth / extendedLogicalWidth,
                              extendedPhysicalWidth / extendedLogicalWidth);

    if (_align == SVG_PRESERVEASPECTRATIO_XMINYMIN || _align == SVG_PRESERVEASPECTRATIO_XMIDYMIN
        || _align == SVG_PRESERVEASPECTRATIO_XMAXYMIN)
    {
      transform.Translate(-extendedLogicalX, -extendedLogicalY);
    }
    else if (_align == SVG_PRESERVEASPECTRATIO_XMINYMID || _align == SVG_PRESERVEASPECTRATIO_XMIDYMID
             || _align == SVG_PRESERVEASPECTRATIO_XMAXYMID)
    {
      transform.Translate(
        -extendedLogicalX,
        -extendedLogicalY
          - (extendedLogicalHeight - extendedPhysicalHeight * extendedLogicalWidth / extendedPhysicalWidth)
              / 2);
    }
    else
    {
      transform.Translate(
        -extendedLogicalX,
        -extendedLogicalY
          - (extendedLogicalHeight - extendedPhysicalHeight * extendedLogicalWidth / extendedPhysicalWidth));
    }

    return transform;
  }

  dom::DOMString SVGPreserveAspectRatioValue::ValueAsString() const noexcept
  {
    auto alignType = [&]() -> dom::DOMStringView
    {
      switch (_align)
      {
        case SVG_PRESERVEASPECTRATIO_NONE:
        {
          return u8"none";
        }
        case SVG_PRESERVEASPECTRATIO_XMINYMIN:
        {
          return u8"xMinYMin";
        }
        case SVG_PRESERVEASPECTRATIO_XMIDYMIN:
        {
          return u8"xMidYMin";
        }
        case SVG_PRESERVEASPECTRATIO_XMAXYMIN:
        {
          return u8"xMaxYMin";
        }
        case SVG_PRESERVEASPECTRATIO_XMINYMID:
        {
          return u8"xMinYMid";
        }
        case SVG_PRESERVEASPECTRATIO_XMIDYMID:
        {
          return u8"xMidYMid";
        }
        case SVG_PRESERVEASPECTRATIO_XMAXYMID:
        {
          return u8"xMaxYMid";
        }
        case SVG_PRESERVEASPECTRATIO_XMINYMAX:
        {
          return u8"xMinYMax";
        }
        case SVG_PRESERVEASPECTRATIO_XMIDYMAX:
        {
          return u8"xMidYMax";
        }
        case SVG_PRESERVEASPECTRATIO_XMAXYMAX:
        {
          return u8"xMaxYMax";
        }
        case SVG_PRESERVEASPECTRATIO_UNKNOWN:
        {
          return u8"unknown";
        }
      };

      krys_assert_msg(false, "Not reached");
      return u8"unknown";
    };

    StringBuilder builder;
    switch (_meetOrSlice)
    {
      default:
      case SVG_MEETORSLICE_UNKNOWN:
      {
        builder.Append(alignType());
        break;
      }
      case SVG_MEETORSLICE_MEET:
      {
        builder.Append(alignType(), u8" meet");
        break;
      }
      case SVG_MEETORSLICE_SLICE:
      {
        builder.Append(alignType(), u8" slice");
        break;
      }
    }
    return krys::move(builder).ToString();
  }
}
