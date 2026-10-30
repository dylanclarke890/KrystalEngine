#include "Krystal.Booey/HTML/HTMLImageLoader.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"

namespace krys::boo::html
{
  HTMLImageLoader::HTMLImageLoader(dom::Element &element) noexcept : ImageLoader(element)
  {
  }

  HTMLImageLoader::~HTMLImageLoader() noexcept = default;

  void HTMLImageLoader::DispatchLoadEvent() noexcept
  {
    // FIXME: Video posters fire no events, and an <object> considers an HTTP error status an error, once
    // they load images.
    bool errorOccurred = Image()->ErrorOccurred();
    (void)dom::EventDispatcher::FireEvent(
      dom::DOMStringAtom(errorOccurred ? EventNames::Error : EventNames::Load), GetElement());
  }

  void HTMLImageLoader::NotifyFinished(CachedResource &resource) noexcept
  {
    krys_debug_assert(Image());
    Ref protect = ShareRef(GetElement());
    ImageLoader::NotifyFinished(resource);
    // FIXME: An <object> renders its fallback content when the load failed, once there are <object> loads.
  }
}
