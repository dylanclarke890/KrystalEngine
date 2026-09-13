#pragma once
#pragma once

#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css::PropertyParserHelpers
{
  enum class AllowedImageType
  {
    None = 0,
    URLFunction = 1 << 0,
    RawStringAsURL = 1 << 1,
    ImageSet = 1 << 2,
    GeneratedImage = 1 << 3
  };
}

KRYS_DEFINE_FLAGS_ENUM_TRAITS(krys::boo::css::PropertyParserHelpers::AllowedImageType, 5uz)

namespace krys::boo::css
{
  class CSSValue;
  class TokenRange;
  struct PropertyParserState;

  namespace PropertyParserHelpers
  {
    // MARK: <image>
    // https://drafts.csswg.org/css-images-4/#image-values

    KRYS_NODISCARD RefPtr<CSSValue>
      ConsumeImage(TokenRange &tokens, PropertyParserState &state,
                   AllowedImageType allowedImageTypes = AllowedImageType::URLFunction
                                                        | AllowedImageType::ImageSet
                                                        | AllowedImageType::GeneratedImage) noexcept;

    KRYS_NODISCARD RefPtr<CSSValue>
      ConsumeImageOrNone(TokenRange &tokens, PropertyParserState &state,
                         AllowedImageType allowedImageTypes = AllowedImageType::URLFunction
                                                              | AllowedImageType::ImageSet
                                                              | AllowedImageType::GeneratedImage) noexcept;
  }
}