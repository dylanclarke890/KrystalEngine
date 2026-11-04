#include "Krystal.Booey/HTML/Images/HTMLImageLoader.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Events/EventNames.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"

namespace krys::boo::html
{
  HTMLImageLoader::HTMLImageLoader(dom::Element &element) noexcept : ImageLoader(element)
  {
  }

  HTMLImageLoader::~HTMLImageLoader() noexcept = default;

  void HTMLImageLoader::DispatchLoadEvent() noexcept
  {
    // FIXME: Video posters fire no events, once they load images.
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
