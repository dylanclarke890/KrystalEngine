#pragma once

#include "Krystal.Booey/SVG/Properties/SVGProperty.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwner.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo::svg::tests
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
