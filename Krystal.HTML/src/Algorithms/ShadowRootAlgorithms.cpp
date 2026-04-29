#include "Krystal.HTML/Algorithms/ShadowRootAlgorithms.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"

namespace Krys::HTML
{
  ExceptionOr<Ref<ShadowRoot>>
    ShadowRootAlgorithms::AttachShadowRoot(Element &element, ShadowRootMode mode, Clonable clonable,
                                           Serializable serializable, DelegatesFocus delegatesFocus,
                                           SlotAssignmentMode slotAssignment,
                                           RawPtr<CustomElementRegistry> registry) noexcept
  {
    // TODO(impl): Implement this method
    return Exception {ExceptionCode::NotSupportedError};
  }
}