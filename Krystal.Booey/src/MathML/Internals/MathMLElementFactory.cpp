#include "Krystal.Booey/MathML/Internals/MathMLElementFactory.hpp"
#include "Krystal.Booey/MathML/MathMLElement.hpp"

namespace krys::boo::mathml
{
  RefPtr<MathMLElement> MathMLElementFactory::TryCreate(dom::Document &document,
                                                        const dom::QualifiedName &name) noexcept
  {
    // TODO(impl): MATHML-ELEMENTS - the element-specific interfaces, if any are added.
    (void)name;
    return CreateRefPtr<MathMLElement>(document);
  }
}
