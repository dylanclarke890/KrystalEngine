#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathConsumer.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathSegType.hpp"

namespace krys::boo::svg
{
  class SVGPathByteStream;
  class SVGPathSource;

  /// @brief Parses SVG path segments from a source and passes them to a consumer.
  class SVGPathParser
  {
  private:
    SVGPathSource &_source;
    SVGPathConsumer &_consumer;
    FloatPoint _controlPoint;
    FloatPoint _currentPoint;
    FloatPoint _subPathPoint;
    PathCoordinateMode _mode {PathCoordinateMode::AbsoluteCoordinates};
    const PathParsingMode _pathParsingMode {PathParsingMode::NormalisedParsing};
    SVGPathSegType _lastCommand {SVGPathSegType::Unknown};
    bool _closePath {true};

  public:
    KRYS_NODISCARD static bool Parse(SVGPathSource &source, SVGPathConsumer &consumer,
                                     PathParsingMode mode = PathParsingMode::NormalisedParsing,
                                     bool checkForInitialMoveTo = true) noexcept;

    KRYS_NODISCARD static bool ParseToByteStream(SVGPathSource &source, SVGPathByteStream &byteStream,
                                                 PathParsingMode mode = PathParsingMode::NormalisedParsing,
                                                 bool checkForInitialMoveTo = true) noexcept;
    KRYS_NODISCARD static bool ParseToString(SVGPathSource &source, dom::DOMString &result,
                                             PathParsingMode mode = PathParsingMode::NormalisedParsing,
                                             bool checkForInitialMoveTo = true) noexcept;

  private:
    SVGPathParser(SVGPathConsumer &consumer, SVGPathSource &source, PathParsingMode parsingMode) noexcept;

    KRYS_NODISCARD bool ParsePathData(bool checkForInitialMoveTo) noexcept;

    KRYS_NODISCARD bool DecomposeArcToCubic(float angle, float rx, float ry, const FloatPoint &start,
                                            const FloatPoint &end, bool largeArcFlag,
                                            bool sweepFlag) noexcept;
    void ParseClosePathSegment() noexcept;
    KRYS_NODISCARD bool ParseMoveToSegment() noexcept;
    KRYS_NODISCARD bool ParseLineToSegment() noexcept;
    KRYS_NODISCARD bool ParseLineToHorizontalSegment() noexcept;
    KRYS_NODISCARD bool ParseLineToVerticalSegment() noexcept;
    KRYS_NODISCARD bool ParseCurveToCubicSegment() noexcept;
    KRYS_NODISCARD bool ParseCurveToCubicSmoothSegment() noexcept;
    KRYS_NODISCARD bool ParseCurveToQuadraticSegment() noexcept;
    KRYS_NODISCARD bool ParseCurveToQuadraticSmoothSegment() noexcept;
    KRYS_NODISCARD bool ParseArcToSegment() noexcept;
  };
}
