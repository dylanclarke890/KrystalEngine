#include "Krystal.Booey/Graphics/Fonts/FontCustomPlatformData.hpp"
#include "Krystal.Booey/CSS/Values/FontFaceSrcValue.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontCreationContext.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontDatabase.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontDescription.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontPlatformData.hpp"
#include "Krystal.Booey/Platform/SharedBuffer.hpp"
#include "Krystal.Core/Text/ASCII.hpp"

namespace krys::boo
{
  RefPtr<FontCustomPlatformData>
    FontCustomPlatformData::Create(SharedBuffer &buffer, const css::CSSOMString &itemInCollection) noexcept
  {
    // FIXME: The named face of a font collection, once collections are matched by name.
    (void)itemInCollection;
    auto bytes = buffer.GetSpan();
    auto data =
      text::FontFileData::Create(List<byte>(reinterpret_cast<const byte *>(bytes.data()),
                                            reinterpret_cast<const byte *>(bytes.data()) + bytes.size()));
    auto face = text::FreeTypeFace::Create(krys::move(data), 0);
    if (!face)
    {
      return nullptr;
    }
    return AdoptRef(*new FontCustomPlatformData(krys::move(face)));
  }

  FontPlatformData
    FontCustomPlatformData::GetFontPlatformData(const FontDescription &description, bool bold, bool italic,
                                                const FontCreationContext &fontCreationContext) const noexcept
  {
    auto size = description.AdjustedSizeForFontFace(fontCreationContext.SizeAdjust());
    const auto &descriptors = fontCreationContext.FontFaceCapabilities();

    // A face that says nothing of a style it has an axis for reaches it, without synthesis (as Chrome).
    // Selection took it to be normal; an `ital` axis is always used, as its value isn't a range a descriptor
    // can limit.
    if (bold && !descriptors.Weight && Face->HasVariationAxis(FontAxisTag('w', 'g', 'h', 't')))
    {
      bold = false;
    }
    if (italic
        && (Face->HasVariationAxis(FontAxisTag('i', 't', 'a', 'l'))
            || (!descriptors.Slope && Face->HasVariationAxis(FontAxisTag('s', 'l', 'n', 't')))))
    {
      italic = false;
    }

    FontPlatformData platformData(InstanceOfFace(*Face, description, descriptors), size, bold, italic,
                                  description.Orientation(), description.WidthVariant(),
                                  description.GetTextRenderingMode());
    platformData.UpdateSizeWithFontSizeAdjust(description.GetFontSizeAdjust(), description.ComputedSize());
    platformData.ApplyCreationContext(description, fontCreationContext);
    return platformData;
  }

  bool FontCustomPlatformData::SupportsFormat(css::CSSOMStringView format) noexcept
  {
    for (auto supported : {u8"truetype", u8"opentype", u8"woff2", u8"woff2-variations", u8"woff-variations",
                           u8"truetype-variations", u8"opentype-variations", u8"woff", u8"svg"})
    {
      if (text::IsASCIICaselessEqual(format, utf8_stringview(supported)))
      {
        return true;
      }
    }

    return false;
  }

  bool FontCustomPlatformData::SupportsTechnology(css::FontTechnology technology) noexcept
  {
    using enum css::FontTechnology;

    switch (technology)
    {
      case ColorCbdt:
      case ColorColrv0:
      case ColorColrv1:
      case ColorSbix:
      case ColorSvg:
      case FeaturesAat:
      case FeaturesOpentype:
      case Palettes:
      case Variations:       return true;
      case FeaturesGraphite:
      case Incremental:
      case Invalid:          return false;
    }

    return false;
  }
}
