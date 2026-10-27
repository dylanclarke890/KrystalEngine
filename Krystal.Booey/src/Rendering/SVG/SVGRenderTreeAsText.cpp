#include "Krystal.Booey/Rendering/SVG/SVGRenderTreeAsText.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/RenderChildIterator.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGContainer.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGImage.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceLinearGradient.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourcePattern.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceRadialGradient.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceSolidColor.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGRoot.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGShape.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGGradientStop.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathUtilities.hpp"
#include "Krystal.Booey/SVG/SVGCircleElement.hpp"
#include "Krystal.Booey/SVG/SVGEllipseElement.hpp"
#include "Krystal.Booey/SVG/SVGLengthContext.hpp"
#include "Krystal.Booey/SVG/SVGLinearGradientElement.hpp"
#include "Krystal.Booey/SVG/SVGLineElement.hpp"
#include "Krystal.Booey/SVG/SVGPathElement.hpp"
#include "Krystal.Booey/SVG/SVGPointList.hpp"
#include "Krystal.Booey/SVG/SVGPolyElement.hpp"
#include "Krystal.Booey/SVG/SVGRadialGradientElement.hpp"
#include "Krystal.Booey/SVG/SVGRectElement.hpp"
#include "Krystal.Booey/SVG/SVGStopElement.hpp"
#include "Krystal.Booey/SVG/SVGURIReference.hpp"
#include "Krystal.Core/Text/ASCII.hpp"

namespace krys::boo
{
  namespace
  {
    // Streams list separators, i.e. ", " in string "a, b, c, d", when it isn't known which item in the list
    // is the first one to be printed, but strings like ", b, c" should still be avoided.
    class TextStreamSeparator
    {
    private:
      char8_t _separator;
      bool _needToSeparate {false};

    public:
      explicit TextStreamSeparator(char8_t separator) noexcept : _separator(separator)
      {
      }

      friend TextStream &operator<<(TextStream &ts, TextStreamSeparator &separator) noexcept
      {
        if (separator._needToSeparate)
        {
          ts << separator._separator;
        }
        else
        {
          separator._needToSeparate = true;
        }
        return ts;
      }
    };

    TextStream &operator<<(TextStream &ts, const DashArray &a) noexcept
    {
      ts << '{';
      bool isFirst = true;
      for (auto value : a)
      {
        if (isFirst)
        {
          isFirst = false;
        }
        else
        {
          ts << u8", ";
        }
        ts << value;
      }
      ts << '}';
      return ts;
    }

    TextStream &operator<<(TextStream &ts, const svg::SVGUnitTypes::SVGUnitType &unitType) noexcept
    {
      ts << svg::SVGPropertyTraits<svg::SVGUnitTypes::SVGUnitType>::ToString(unitType);
      return ts;
    }

    TextStream &operator<<(TextStream &ts, const svg::SVGSpreadMethodType &type) noexcept
    {
      ts << text::ToASCIIUpper(svg::SVGPropertyTraits<svg::SVGSpreadMethodType>::ToString(type));
      return ts;
    }

    template <typename ValueType>
    void WriteNameValuePair(TextStream &ts, utf8_stringview name, const ValueType &value) noexcept
    {
      ts << u8" [" << name << '=' << value << ']';
    }

    template <typename ValueType>
    void WriteNameAndQuotedValue(TextStream &ts, utf8_stringview name, const ValueType &value) noexcept
    {
      ts << u8" [" << name << u8"=\"" << value << u8"\"]";
    }

    void WriteIfNotEmpty(TextStream &ts, utf8_stringview name, utf8_stringview value) noexcept
    {
      if (!value.empty())
      {
        WriteNameValuePair(ts, name, value);
      }
    }

    template <typename ValueType>
    void WriteIfNotDefault(TextStream &ts, utf8_stringview name, ValueType value,
                           ValueType defaultValue) noexcept
    {
      if (value != defaultValue)
      {
        WriteNameValuePair(ts, name, value);
      }
    }

    void WriteSVGPaintingResource(TextStream &ts, const LegacyRenderSVGResource &resource) noexcept
    {
      if (auto *solidColorResource = DynamicDowncast<LegacyRenderSVGResourceSolidColor>(resource))
      {
        ts << u8"[type=SOLID] [color=" << solidColorResource->GetColor() << ']';
        return;
      }

      auto resourceType = resource.ResourceType();
      if (resourceType == RenderSVGResourceType::PatternResourceType)
      {
        ts << u8"[type=PATTERN]";
      }
      else if (resourceType == RenderSVGResourceType::LinearGradientResourceType)
      {
        ts << u8"[type=LINEAR-GRADIENT]";
      }
      else if (resourceType == RenderSVGResourceType::RadialGradientResourceType)
      {
        ts << u8"[type=RADIAL-GRADIENT]";
      }

      // All other resources derive from LegacyRenderSVGResourceContainer.
      // FIXME: This should use a safe cast.
      const auto &container = static_cast<const LegacyRenderSVGResourceContainer &>(resource);
      ts << u8" [id=\"" << container.GetElement().Id() << u8"\"]";
    }

    void WriteSVGFillPaintingResource(TextStream &ts, const RenderElement &renderer,
                                      const LegacyRenderSVGResource &fillPaintingResource) noexcept
    {
      TextStreamSeparator s(' ');
      ts << u8" [fill={" << s;
      WriteSVGPaintingResource(ts, fillPaintingResource);

      const auto &style = renderer.Style();
      WriteIfNotDefault(ts, u8"opacity", style.FillOpacity().Value.Value, 1.0f);
      WriteIfNotDefault(ts, u8"fill rule", style.GetFillRule(), WindRule::NonZero);
      ts << u8"}]";
    }

    void WriteSVGStrokePaintingResource(TextStream &ts, const RenderElement &renderer,
                                        const LegacyRenderSVGResource &strokePaintingResource,
                                        const svg::SVGGraphicsElement &shape) noexcept
    {
      TextStreamSeparator s(' ');
      ts << u8" [stroke={" << s;
      WriteSVGPaintingResource(ts, strokePaintingResource);

      const auto &style = renderer.Style();

      svg::SVGLengthContext lengthContext(&shape);
      double dashOffset = lengthContext.ValueForLength(style.StrokeDashOffset(), style::ZoomNeeded {});
      double strokeWidth = lengthContext.ValueForLength(style.GetStrokeWidth(), style::ZoomNeeded {});
      auto dashArray = DashArray::map(style.StrokeDashArray(), [&](const auto &length) -> DashArrayElement
                                      { return lengthContext.ValueForLength(length, style::ZoomNeeded {}); });

      WriteIfNotDefault(ts, u8"opacity", style.StrokeOpacity().Value.Value, 1.0f);
      WriteIfNotDefault(ts, u8"stroke width", strokeWidth, 1.0);
      WriteIfNotDefault(ts, u8"miter limit", style.StrokeMiterLimit().Value.Value, 4.0f);
      WriteIfNotDefault(ts, u8"line cap", style.CapStyle(), LineCap::Butt);
      WriteIfNotDefault(ts, u8"line join", style.JoinStyle(), LineJoin::Miter);
      WriteIfNotDefault(ts, u8"dash offset", dashOffset, 0.0);
      if (!dashArray.isEmpty())
      {
        WriteNameValuePair(ts, u8"dash array", dashArray);
      }

      if (auto *element = DynamicDowncast<svg::SVGGeometryElement>(shape))
      {
        double pathLength = element->PathLength();
        WriteIfNotDefault(ts, u8"path length", pathLength, 0.0);
      }

      ts << u8"}]";
    }
  }

  void WriteSVGPaintingFeatures(TextStream &ts, const RenderElement &renderer,
                                EnumSet<RenderAsTextFlag>) noexcept
  {
    auto &style = renderer.Style();

    if (!renderer.LocalTransform().IsIdentity())
    {
      WriteNameValuePair(ts, u8"transform", renderer.LocalTransform());
    }
    WriteIfNotDefault(ts, u8"image rendering", style.GetImageRendering(),
                      style::ComputedStyle::InitialImageRendering());
    WriteIfNotDefault(ts, u8"opacity", style.GetOpacity().Value.Value, 1.0f);

    if (auto *shape = DynamicDowncast<LegacyRenderSVGShape>(renderer))
    {
      Color fallbackColor;
      if (auto *strokePaintingResource = LegacyRenderSVGResource::StrokePaintingResource(
            const_cast<LegacyRenderSVGShape &>(*shape), shape->Style(), fallbackColor))
      {
        WriteSVGStrokePaintingResource(ts, renderer, *strokePaintingResource, shape->GraphicsElement());
      }

      if (auto *fillPaintingResource = LegacyRenderSVGResource::FillPaintingResource(
            const_cast<LegacyRenderSVGShape &>(*shape), shape->Style(), fallbackColor))
      {
        WriteSVGFillPaintingResource(ts, renderer, *fillPaintingResource);
      }

      WriteIfNotDefault(ts, u8"clip rule", style.ClipRule(), WindRule::NonZero);
    }

    auto writeMarker = [&](utf8_stringview name, const style::SVGMarkerResource &value)
    {
      auto *element = renderer.GetElement();
      if (!element)
      {
        return;
      }

      auto fragment = svg::SVGURIReference::FragmentIdentifierFromIRIString(value, element->NodeDocument());
      WriteIfNotEmpty(ts, name, fragment.View());
    };

    writeMarker(u8"start marker", style.MarkerStart());
    writeMarker(u8"middle marker", style.MarkerMid());
    writeMarker(u8"end marker", style.MarkerEnd());
  }

  namespace
  {
    TextStream &WritePositionAndStyle(TextStream &ts, const RenderElement &renderer,
                                      EnumSet<RenderAsTextFlag> behavior = {}) noexcept
    {
      if (behavior.Contains(RenderAsTextFlag::ShowSVGGeometry))
      {
        if (auto *box = DynamicDowncast<RenderBox>(renderer))
        {
          ts << ' ' << EnclosingIntRect(box->FrameRect());
        }
        ts << u8" clipped";
      }

      ts << ' ' << EnclosingIntRect(renderer.AbsoluteClippedOverflowRectForRenderTreeAsText());

      WriteSVGPaintingFeatures(ts, renderer, behavior);
      return ts;
    }
  }

  void WriteSVGGraphicsElement(TextStream &ts, const svg::SVGGraphicsElement &svgElement) noexcept
  {
    svg::SVGLengthContext lengthContext(&svgElement);

    if (auto *element = DynamicDowncast<svg::SVGRectElement>(svgElement))
    {
      WriteNameValuePair(ts, u8"x", element->X().Value(lengthContext));
      WriteNameValuePair(ts, u8"y", element->Y().Value(lengthContext));
      WriteNameValuePair(ts, u8"width", element->Width().Value(lengthContext));
      WriteNameValuePair(ts, u8"height", element->Height().Value(lengthContext));
    }
    else if (auto *line = DynamicDowncast<svg::SVGLineElement>(svgElement))
    {
      WriteNameValuePair(ts, u8"x1", line->X1().Value(lengthContext));
      WriteNameValuePair(ts, u8"y1", line->Y1().Value(lengthContext));
      WriteNameValuePair(ts, u8"x2", line->X2().Value(lengthContext));
      WriteNameValuePair(ts, u8"y2", line->Y2().Value(lengthContext));
    }
    else if (auto *ellipse = DynamicDowncast<svg::SVGEllipseElement>(svgElement))
    {
      WriteNameValuePair(ts, u8"cx", ellipse->Cx().Value(lengthContext));
      WriteNameValuePair(ts, u8"cy", ellipse->Cy().Value(lengthContext));
      WriteNameValuePair(ts, u8"rx", ellipse->Rx().Value(lengthContext));
      WriteNameValuePair(ts, u8"ry", ellipse->Ry().Value(lengthContext));
    }
    else if (auto *circle = DynamicDowncast<svg::SVGCircleElement>(svgElement))
    {
      WriteNameValuePair(ts, u8"cx", circle->Cx().Value(lengthContext));
      WriteNameValuePair(ts, u8"cy", circle->Cy().Value(lengthContext));
      WriteNameValuePair(ts, u8"r", circle->R().Value(lengthContext));
    }
    else if (auto *poly = DynamicDowncast<svg::SVGPolyElement>(svgElement))
    {
      WriteNameAndQuotedValue(ts, u8"points", poly->Points().ValueAsString());
    }
    else if (auto *path = DynamicDowncast<svg::SVGPathElement>(svgElement))
    {
      dom::DOMString pathString;
      // FIXME: We should switch to UnalteredParsing here - this will affect the path dumping output of dozens
      // of tests.
      (void)svg::BuildStringFromByteStream(path->PathByteStream(), pathString,
                                           svg::PathParsingMode::NormalisedParsing);
      WriteNameAndQuotedValue(ts, u8"data", pathString);
    }
    else
    {
      krys_debug_assert(false);
    }
  }

  namespace
  {
    TextStream &operator<<(TextStream &ts, const LegacyRenderSVGShape &shape) noexcept
    {
      WritePositionAndStyle(ts, shape);
      WriteSVGGraphicsElement(ts, shape.GraphicsElement());
      return ts;
    }

    enum class WriteIndentOrNot : bool
    {
      No,
      Yes
    };

    void WriteStandardPrefix(TextStream &ts, const RenderObject &object, EnumSet<RenderAsTextFlag> behavior,
                             WriteIndentOrNot writeIndent = WriteIndentOrNot::Yes) noexcept
    {
      if (writeIndent == WriteIndentOrNot::Yes)
      {
        ts << Indent;
      }

      ts << object.RenderName();

      if (behavior.Contains(RenderAsTextFlag::ShowAddresses))
      {
        ts << ' ' << static_cast<const void *>(&object);
      }

      if (object.GetNode())
      {
        ts << u8" {" << object.GetNode()->NodeName() << '}';
      }

      WriteDebugInfo(ts, object, behavior);
    }

    void WriteChildren(TextStream &ts, const RenderElement &parent,
                       EnumSet<RenderAsTextFlag> behavior) noexcept
    {
      TextStream::IndentScope indentScope(ts);

      for (const auto &child : ChildrenOfType<RenderObject>(parent))
      {
        Write(ts, child, behavior);
      }
    }
  }

  namespace
  {
    void WriteCommonGradientProperties(TextStream &ts, svg::SVGSpreadMethodType spreadMethod,
                                       const AffineTransform &gradientTransform,
                                       svg::SVGUnitTypes::SVGUnitType gradientUnits) noexcept
    {
      WriteNameValuePair(ts, u8"gradientUnits", gradientUnits);

      if (spreadMethod != svg::SVGSpreadMethodPad)
      {
        ts << u8" [spreadMethod=" << spreadMethod << ']';
      }

      if (!gradientTransform.IsIdentity())
      {
        ts << u8" [gradientTransform=" << gradientTransform << ']';
      }
    }
  }

  void WriteSVGResourceContainer(TextStream &ts, const LegacyRenderSVGResourceContainer &resource,
                                 EnumSet<RenderAsTextFlag> behavior) noexcept
  {
    WriteStandardPrefix(ts, resource, behavior);

    auto id = resource.GetElement().Id();
    WriteNameAndQuotedValue(ts, u8"id", id);

    // FIXME: Maskers, filters, clippers and markers, with clipping, masking and markers (B11f) and filters
    // (B11h).
    if (auto *pattern = DynamicDowncast<const LegacyRenderSVGResourcePattern>(resource))
    {
      // Dump final results that are used for rendering. No use in asking SVGPatternElement for its
      // patternUnits(), as it may link to other patterns using xlink:href, we need to build the full
      // inheritance chain, aka. collectPatternProperties()
      svg::PatternAttributes attributes;
      pattern->CollectPatternAttributes(attributes);

      WriteNameValuePair(ts, u8"patternUnits", attributes.PatternUnits());
      WriteNameValuePair(ts, u8"patternContentUnits", attributes.PatternContentUnits());

      AffineTransform transform = attributes.PatternTransform();
      if (!transform.IsIdentity())
      {
        ts << u8" [patternTransform=" << transform << ']';
      }
      ts << '\n';
    }
    else if (auto *linearGradient = DynamicDowncast<const LegacyRenderSVGResourceLinearGradient>(resource))
    {
      // Dump final results that are used for rendering. No use in asking SVGGradientElement for its
      // gradientUnits(), as it may link to other gradients using xlink:href, we need to build the full
      // inheritance chain, aka. collectGradientProperties()
      svg::LinearGradientAttributes attributes;
      (void)linearGradient->LinearGradientElement().CollectGradientAttributes(attributes);
      WriteCommonGradientProperties(ts, attributes.SpreadMethod(), attributes.GradientTransform(),
                                    attributes.GradientUnits());

      ts << u8" [start=" << linearGradient->StartPoint(attributes) << u8"] [end="
         << linearGradient->EndPoint(attributes) << u8"]\n";
    }
    else if (auto *radialGradient = DynamicDowncast<const LegacyRenderSVGResourceRadialGradient>(resource))
    {
      // Dump final results that are used for rendering. No use in asking SVGGradientElement for its
      // gradientUnits(), as it may link to other gradients using xlink:href, we need to build the full
      // inheritance chain, aka. collectGradientProperties()
      svg::RadialGradientAttributes attributes;
      (void)radialGradient->RadialGradientElement().CollectGradientAttributes(attributes);
      WriteCommonGradientProperties(ts, attributes.SpreadMethod(), attributes.GradientTransform(),
                                    attributes.GradientUnits());

      FloatPoint focalPoint = radialGradient->FocalPoint(attributes);
      FloatPoint centerPoint = radialGradient->CenterPoint(attributes);
      float radius = radialGradient->Radius(attributes);
      float focalRadius = radialGradient->FocalRadius(attributes);

      ts << u8" [center=" << centerPoint << u8"] [focal=" << focalPoint << u8"] [radius=" << radius
         << u8"] [focalRadius=" << focalRadius << u8"]\n";
    }
    else
    {
      ts << '\n';
    }
    WriteChildren(ts, resource, behavior);
  }

  void WriteSVGGradientStop(TextStream &ts, const RenderSVGGradientStop &stop,
                            EnumSet<RenderAsTextFlag> behavior) noexcept
  {
    WriteStandardPrefix(ts, stop, behavior);

    ts << u8" [offset=" << stop.GetElement().Offset() << u8"] [color="
       << stop.GetElement().StopColorIncludingOpacity() << u8"]\n";
  }

  void WriteSVGContainer(TextStream &ts, const LegacyRenderSVGContainer &container,
                         EnumSet<RenderAsTextFlag> behavior) noexcept
  {
    // Currently RenderSVGResourceFilterPrimitive has no meaningful output.
    if (container.IsLegacyRenderSVGResourceFilterPrimitive())
    {
      return;
    }
    WriteStandardPrefix(ts, container, behavior);
    WritePositionAndStyle(ts, container, behavior);
    ts << '\n';
    WriteResources(ts, container, behavior);
    WriteChildren(ts, container, behavior);
  }

  void WriteSVGImage(TextStream &ts, const LegacyRenderSVGImage &image,
                     EnumSet<RenderAsTextFlag> behavior) noexcept
  {
    WriteStandardPrefix(ts, image, behavior);
    WritePositionAndStyle(ts, image, behavior);
    ts << '\n';
    WriteResources(ts, image, behavior);
  }

  void Write(TextStream &ts, const LegacyRenderSVGShape &shape, EnumSet<RenderAsTextFlag> behavior) noexcept
  {
    WriteStandardPrefix(ts, shape, behavior);
    ts << shape << '\n';
    WriteResources(ts, shape, behavior);
  }

  void Write(TextStream &ts, const LegacyRenderSVGRoot &root, EnumSet<RenderAsTextFlag> behavior) noexcept
  {
    WriteStandardPrefix(ts, root, behavior);
    WritePositionAndStyle(ts, root, behavior);
    ts << '\n';
    WriteChildren(ts, root, behavior);
  }

  void WriteResources(TextStream &, const RenderObject &, EnumSet<RenderAsTextFlag>) noexcept
  {
    // FIXME: The masker, clipper and filter the renderer references, with clipping and masking (B11f) and
    // filters (B11h).
  }
}
