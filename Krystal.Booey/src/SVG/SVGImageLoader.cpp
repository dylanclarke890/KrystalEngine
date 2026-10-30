#include "Krystal.Booey/SVG/SVGImageLoader.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/Loader/Cache/CachedImage.hpp"
#include "Krystal.Booey/SVG/SVGImageElement.hpp"

namespace krys::boo::svg
{
  SVGImageLoader::SVGImageLoader(SVGImageElement &element) noexcept : ImageLoader(element)
  {
  }

  SVGImageLoader::~SVGImageLoader() noexcept = default;

  void SVGImageLoader::DispatchLoadEvent() noexcept
  {
    if (Image()->ErrorOccurred())
    {
      (void)dom::EventDispatcher::FireEvent(dom::DOMStringAtom(EventNames::Error), GetElement());
    }
    else
    {
      Ref<SVGImageElement> element = ShareRef(Downcast<SVGImageElement>(Downcast<SVGElement>(GetElement())));
      element->SendLoadEventIfPossible();
    }
  }
}
