#pragma once

#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  struct SerialisationContext;
  class Value;
}

namespace krys::boo::style
{
  struct ExtractorState;

  /// @brief Extracts a property's computed value from a style.
  class ExtractorGenerated
  {
  public:
    /// @brief Extracts value for `css::PropertyId` from `ExtractorState` as a `css::Value`.
    KRYS_NODISCARD static RefPtr<css::Value> ExtractValue(ExtractorState &, css::PropertyId) noexcept;

    /// @brief Extracts value serialisation for `css::PropertyId` from `ExtractorState`, appending result to
    /// `builder`.
    static void ExtractValueSerialisation(ExtractorState &, StringBuilder &builder,
                                          const css::SerialisationContext &, css::PropertyId) noexcept;
  };
}
