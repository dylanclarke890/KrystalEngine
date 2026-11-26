#include "Krystal.Web/Html/Images/HTMLImageLoader.hpp"
#include "Krystal.Web/Dom/Algorithms/EventDispatcher.hpp"
#include "Krystal.Web/Dom/Events/EventNames.hpp"
#include "Krystal.Web/Dom/Nodes/Element.hpp"

namespace krys::web::html
{
  HTMLImageLoader::HTMLImageLoader(web::Element &element) noexcept : ImageLoader(element)
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
