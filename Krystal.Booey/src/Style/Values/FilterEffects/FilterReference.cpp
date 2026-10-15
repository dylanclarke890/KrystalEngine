#include "Krystal.Booey/Style/Values/FilterEffects/FilterReference.hpp"

#include "Krystal.Booey/CSS/Values/FilterEffects/FilterReference.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Rendering/Style/ReferenceFilterOperation.hpp"
#include "Krystal.Booey/Style/Seam.hpp"

namespace krys::boo::style
{
  KRYS_NODISCARD css::FilterReference ToCSSFilterReference(const ReferenceFilterOperation &operation,
                                                           const RenderStyle &style) noexcept
  {
    return {.Url = ConvertToCSS(operation.Url(), style)};
  }

  Ref<FilterOperation> CreateFilterOperation(const css::FilterReference &filter,
                                             const BuilderState &state) noexcept
  {
    auto url = ToStyleWithScriptExecutionContext(filter.Url, state.GetDocument());

    // FIXME: Unify all the fragment accessing/construction.
    utf8_stringview resolved = url.Resolved.String();
    auto fragment = resolved.starts_with(u8'#') ? StringAtom {resolved.substr(1)}
                                                : StringAtom {url.Resolved.FragmentIdentifier()};

    return ReferenceFilterOperation::Create(krys::move(url), krys::move(fragment));
  }
}
