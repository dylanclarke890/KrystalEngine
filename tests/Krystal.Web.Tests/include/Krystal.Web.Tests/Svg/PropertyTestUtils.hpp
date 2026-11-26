#pragma once

#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Web/Svg/Properties/SVGProperty.hpp"
#include "Krystal.Web/Svg/Properties/SVGPropertyOwner.hpp"

namespace krys::web::svg::tests
{
  /// @brief An owner that records the properties whose changes are committed to it, standing in for an
  /// element.
  struct RecordingPropertyOwner final : public SVGPropertyOwner
  {
    List<SVGProperty *> Committed;

    void CommitPropertyChange(SVGProperty *property) noexcept override
    {
      Committed.push_back(property);
    }
  };
}
