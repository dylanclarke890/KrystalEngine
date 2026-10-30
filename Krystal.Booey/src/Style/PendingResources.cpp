#include "Krystal.Booey/Style/PendingResources.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceLoader.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Booey/Rendering/Style/StyleImage.hpp"

namespace krys::boo::style
{
  namespace
  {
    // <https://html.spec.whatwg.org/multipage/urls-and-fetching.html#cors-settings-attributes>
    enum class LoadPolicy : uint8
    {
      CORS,
      NoCORS,
      Anonymous
    };

    void LoadPendingImage(dom::Document &document, const StyleImage *styleImage, const dom::Element *,
                          LoadPolicy = LoadPolicy::NoCORS) noexcept
    {
      if (!styleImage || !styleImage->IsPending())
      {
        return;
      }

      // FIXME: Skip the content security policy check in user agent shadow trees, and fetch mask and
      // shape-outside images in CORS or anonymous mode (useAnonymousModeWhenFetchingMaskImages), once there
      // are those.
      ResourceLoaderOptions options = CachedResourceLoader::DefaultCachedResourceOptions();

      const_cast<StyleImage &>(*styleImage).Load(document.GetCachedResourceLoader(), options);
    }
  }

  void LoadPendingResources(RenderStyle &style, dom::Document &document, const dom::Element *element) noexcept
  {
    for (auto &backgroundLayer : style.GetBackgroundLayers().UsedValues())
    {
      LoadPendingImage(document, backgroundLayer.GetImage().TryStyleImage().get(), element);
    }

    if (auto *contentData = style.GetContent().TryData())
    {
      for (auto &contentItem : contentData->List)
      {
        if (auto *image = std::get_if<Content::Image>(&contentItem))
        {
          LoadPendingImage(document, image->Value.Value.get(), element);
        }
      }
    }

    if (auto cursorImages = style.GetCursor().Images)
    {
      for (auto &cursorImage : *cursorImages)
      {
        LoadPendingImage(document, cursorImage.Image.get(), element);
      }
    }

    LoadPendingImage(document, style.ListStyleImage().TryStyleImage().get(), element);
    LoadPendingImage(document, style.GetBorderImageSource().TryStyleImage().get(), element);
    LoadPendingImage(document, style.GetMaskBorderSource().TryStyleImage().get(), element);

    // Masking operations may be sensitive to timing attacks that can be used to reveal the pixel data of
    // the image used as the mask. As a means to mitigate such attacks CSS mask images and shape-outside
    // images are retrieved in "Anonymous" mode, which uses a potentially CORS-enabled fetch.
    for (auto &maskLayer : style.GetMaskLayers().UsedValues())
    {
      LoadPendingImage(document, maskLayer.GetImage().TryStyleImage().get(), element, LoadPolicy::CORS);
    }

    if (RefPtr<StyleImage> shapeValueImage = style.GetShapeOutside().GetImage())
    {
      LoadPendingImage(document, shapeValueImage.get(), element, LoadPolicy::Anonymous);
    }

    // Are there other pseudo-elements that need resource loading?
    if (auto *firstLineStyle = style.GetCachedPseudoStyle({PseudoElementType::FirstLine}))
    {
      LoadPendingResources(*firstLineStyle, document, element);
    }
  }
}
