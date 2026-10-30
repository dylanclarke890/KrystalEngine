#include "Krystal.Booey/CSS/Values/PathValue.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesValueVisitation.hpp"

namespace krys::boo::css
{
  Ref<PathValue> PathValue::Create(PathFunction path) noexcept
  {
    return AdoptRef(*new PathValue(krys::move(path)));
  }

  PathValue::PathValue(PathFunction &&path) noexcept : Value(ValueType::Path), _path(krys::move(path))
  {
  }

  CSSOMString PathValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    return Serialise(context, _path);
  }

  bool PathValue::Equals(const PathValue &other) const noexcept
  {
    return _path == other._path;
  }

  IterationStatus
    PathValue::CustomVisitChildren(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept
  {
    return VisitValueChildren(func, _path);
  }
}
