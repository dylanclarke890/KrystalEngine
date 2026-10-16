#include "Krystal.Booey/DOM/Iterators/ComposedTreeIterator.hpp"
#include <format>

namespace krys::boo::dom
{
  DOMString ComposedTreeAsText(ContainerNode &root, ComposedTreeAsTextMode mode) noexcept
  {
    DOMString stream;
    auto descendants = ComposedTreeDescendants(root);
    for (auto it = descendants.begin(), end = descendants.end(); it != end; ++it)
    {
      stream.append(it.Depth() * 2, u8' ');

      auto withPointer = [&](const Node &node)
      {
        if (mode == ComposedTreeAsTextMode::WithPointers)
        {
          auto pointer = std::format(" {}", static_cast<const void *>(&node));
          stream.append(pointer.begin(), pointer.end());
        }
      };

      if (Is<Text>(*it))
      {
        stream += u8"#text";
        withPointer(*it);
        stream += u8'\n';
        continue;
      }
      auto &element = Downcast<Element>(*it);
      stream += element.LocalName().View();
      if (TreeQueries::GetShadowRoot(element))
      {
        stream += u8" (shadow root)";
      }
      withPointer(element);
      stream += u8'\n';
    }
    return stream;
  }
}
