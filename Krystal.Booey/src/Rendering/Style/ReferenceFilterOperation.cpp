#include "Krystal.Booey/Rendering/Style/ReferenceFilterOperation.hpp"

namespace krys::boo::style
{
  ReferenceFilterOperation::ReferenceFilterOperation(URL &&url, StringAtom &&fragment) noexcept
      : FilterOperation(Type::Reference), _url(krys::move(url)), _fragment(krys::move(fragment))
  {
  }

  ReferenceFilterOperation::~ReferenceFilterOperation() noexcept = default;

  bool ReferenceFilterOperation::Equals(const FilterOperation &operation) const noexcept
  {
    if (!IsSameType(operation))
    {
      return false;
    }

    return _url == Downcast<ReferenceFilterOperation>(operation)._url;
  }

  bool ReferenceFilterOperation::IsIdentity() const noexcept
  {
    // Answering this question requires access to the renderer and the referenced filterElement.
    krys_debug_assert_msg(false, "Unreachable");
    return false;
  }

  IntOutsets ReferenceFilterOperation::Outsets() const noexcept
  {
    // Answering this question requires access to the renderer and the referenced filterElement.
    krys_debug_assert_msg(false, "Unreachable");
    return {};
  }
}
