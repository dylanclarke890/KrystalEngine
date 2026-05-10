#include "Krystal.HTML/Node/NamedNodeMap.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"

// TODO(impl): Implement this class
namespace Krys::HTML
{
  size_t NamedNodeMap::Length() noexcept
  {
    return 0uz;
  }

  RawPtr<Attr> NamedNodeMap::Item(size_t index) noexcept
  {
    return nullptr;
  }

  RawPtr<Attr> NamedNodeMap::NamedItem(const DOMString &qualifiedName) noexcept
  {
    return nullptr;
  }

  RawPtr<Attr> NamedNodeMap::NamedItemNS(const DOMString &attrNamespace, const DOMString &localName) noexcept
  {
    return nullptr;
  }

  RawPtr<Attr> NamedNodeMap::SetNamedItem(Attr &attr) noexcept
  {
    return nullptr;
  }

  RawPtr<Attr> NamedNodeMap::SetNamedItemNS(Attr &attr) noexcept
  {
    return nullptr;
  }

  ExceptionOr<Ref<Attr>> NamedNodeMap::RemoveNamedItem(const DOMString &qualifiedName) noexcept
  {
    return Exception {ExceptionCode::NotSupportedError};
  }

  ExceptionOr<Ref<Attr>> NamedNodeMap::RemoveNamedItemNS(const DOMString &attrNamespace,
                                                         const DOMString &localName) noexcept
  {
    return Exception {ExceptionCode::NotSupportedError};
  }
}