#include "Krystal.Booey/Style/ResolvedScopedName.hpp"

#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.Booey/Style/ScopedName.hpp"
#include "Krystal.Core/Assert.hpp"

namespace krys::boo::style
{
  ResolvedScopedName ResolvedScopedName::CreateFromScopedName(const dom::Element &element,
                                                              const ScopedName &scopedName) noexcept
  {
    const auto *scope = Scope::ForOrdinal(element, scopedName.ScopeOrdinal);
    krys_debug_assert(scope);

    return ResolvedScopedName {scopedName.Name, scope ? scope->Identifier() : ScopeIdentifier {}};
  }
}
