#include "Krystal.Booey/Html/Images/HTMLImageLoader.hpp"
#include "Krystal.Booey/Dom/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/Dom/Events/EventNames.hpp"
#include "Krystal.Booey/Dom/Nodes/Element.hpp"

namespace krys::boo::html
{
  HTMLImageLoader::HTMLImageLoader(boo::Element &element) noexcept : ImageLoader(element)
  {
  }

  HTMLImageLoader::~HTMLImageLoader() noexcept = default;

  void HTMLImageLoader::DispatchLoadEvent() noexcept
  {
    // FIXME: Video posters fire no events, once they load images.
    bool errorOccurred = Image()->ErrorOccurred();
    (void)dom::EventDispatcher::FireEvent(
      DOMStringAtom(errorOccurred ? event_names::Error : event_names::Load), GetElement());
  }

  void HTMLImageLoader::NotifyFinished(CachedResource &resource) noexcept
  {
    krys_debug_assert(Image());
    Ref protect = ShareRef(GetElement());
    ImageLoader::NotifyFinished(resource);
    // FIXME: An <object> renders its fallback content when the load failed, once there are <object> loads.
  }
}
