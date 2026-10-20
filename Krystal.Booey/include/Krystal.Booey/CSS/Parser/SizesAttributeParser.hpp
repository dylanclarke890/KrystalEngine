#pragma once

#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQuery.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo::css
{
  struct ParserContext;
  enum class LengthUnit : uint8;

  /// @brief Evaluates a `sizes` attribute: the length of its first entry whose media condition matches, or
  /// `100vw`.
  /// @see https://html.spec.whatwg.org/#sizes-attributes
  class SizesAttributeParser
  {
  private:
    WeakPtr<const dom::Document> _document;
    List<mq::MediaQueryResult> _dynamicMediaQueryResults;
    float _length {0};
    bool _lengthWasSet {false};
    bool _isValid {false};

  public:
    SizesAttributeParser(CSSOMStringView attribute, const dom::Document &document) noexcept;

    KRYS_NODISCARD float Length() const noexcept;

    KRYS_NODISCARD static float ComputeLength(double value, LengthUnit unit,
                                              const dom::Document &document) noexcept;

    /// @brief The media conditions whose result depends on the viewport or the user's preferences.
    KRYS_NODISCARD const List<mq::MediaQueryResult> &DynamicMediaQueryResults() const noexcept
    {
      return _dynamicMediaQueryResults;
    }

  private:
    KRYS_NODISCARD bool Parse(TokenRange range, const ParserContext &context) noexcept;
    KRYS_NODISCARD float EffectiveSize() const noexcept;
    KRYS_NODISCARD Maybe<float> CalculateLengthInPixels(TokenRange range) const noexcept;
    KRYS_NODISCARD bool MediaConditionMatches(const mq::MediaQuery &mediaCondition) const noexcept;
    KRYS_NODISCARD float EffectiveSizeDefaultValue() const noexcept;
  };
}
