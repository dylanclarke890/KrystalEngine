#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/ComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include <algorithm>

namespace krys::boo::css
{
  namespace
  {
    void AppendIfNotContains(List<PropertyId> &list, PropertyId property) noexcept
    {
      if (std::ranges::find(list, property) == list.end())
      {
        list.push_back(property);
      }
    }
  }

  // MARK: - Computed Style Dependencies

  void ComputedStyleDependenciesCollector<LengthUnit>::operator()(ComputedStyleDependencies &dependencies,
                                                                  LengthUnit lengthUnit) noexcept
  {
    using enum LengthUnit;

    switch (lengthUnit)
    {
      case Rcap:
      case Rch:
      case Rex:
      case Ric:
      case Rem:
      {
        AppendIfNotContains(dependencies.RootProperties, PropertyId::FontSize);
        break;
      }
      case Rlh:
      {
        AppendIfNotContains(dependencies.RootProperties, PropertyId::FontSize);
        AppendIfNotContains(dependencies.RootProperties, PropertyId::LineHeight);
        break;
      }
      case Em:
      case QuirkyEm:
      case Ex:
      case Cap:
      case Ch:
      case Ic:
      {
        AppendIfNotContains(dependencies.Properties, PropertyId::FontSize);
        break;
      }
      case Lh:
      {
        AppendIfNotContains(dependencies.Properties, PropertyId::FontSize);
        AppendIfNotContains(dependencies.Properties, PropertyId::LineHeight);
        break;
      }
      case Cqw:
      case Cqh:
      case Cqi:
      case Cqb:
      case Cqmin:
      case Cqmax:
      {
        dependencies.ContainerDimensions = true;
        break;
      }
      case Vw:
      case Vh:
      case Vmin:
      case Vmax:
      case Vb:
      case Vi:
      case Svw:
      case Svh:
      case Svmin:
      case Svmax:
      case Svb:
      case Svi:
      case Lvw:
      case Lvh:
      case Lvmin:
      case Lvmax:
      case Lvb:
      case Lvi:
      case Dvw:
      case Dvh:
      case Dvmin:
      case Dvmax:
      case Dvb:
      case Dvi:
      {
        dependencies.ViewportDimensions = true;
        break;
      }
      case Px:
      case Cm:
      case Mm:
      case In:
      case Pt:
      case Pc:
      case Q:
      {
        break;
      }
    }
  }
}
