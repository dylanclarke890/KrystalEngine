#include "Krystal.Booey/HTML/HTMLPictureElement.hpp"
#include "Krystal.Booey/HTML/HTMLImageElement.hpp"

namespace krys::boo::html
{
  namespace
  {
    KRYS_NODISCARD List<Ref<HTMLImageElement>> ImageChildren(const HTMLPictureElement &picture) noexcept
    {
      List<Ref<HTMLImageElement>> images;
      for (auto *child = picture.FirstChild(); child; child = child->NextSibling())
      {
        if (auto *image = DynamicDowncast<HTMLImageElement>(*child))
        {
          images.push_back(ShareRef(*image));
        }
      }
      return images;
    }
  }

  HTMLPictureElement::HTMLPictureElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Picture)
  {
  }

  void HTMLPictureElement::SourcesChanged() noexcept
  {
    for (auto &element : ImageChildren(*this))
    {
      element->SelectImageSource(RelevantMutation::Yes);
    }
  }

  void HTMLPictureElement::SourceDimensionAttributesChanged(const HTMLSourceElement &sourceElement) noexcept
  {
    for (auto &element : ImageChildren(*this))
    {
      if (&sourceElement == element->SourceElement())
      {
        element->InvalidateAttributeMapping();
      }
    }
  }
}
