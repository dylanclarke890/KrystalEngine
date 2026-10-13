#include "Krystal.Booey/CSS/Values/FontFaceSrcValue.hpp"
#include "Krystal.Booey/CSS/Markup.hpp"
#include "Krystal.Booey/CSS/Values/Types.hpp"

namespace krys::boo::css
{
  FontFaceSrcLocalValue::FontFaceSrcLocalValue(CSSOMStringAtom &&fontFaceName) noexcept
      : Value(ValueType::FontFaceSrcLocal), _fontFaceName(krys::move(fontFaceName))
  {
  }

  Ref<FontFaceSrcLocalValue> FontFaceSrcLocalValue::Create(CSSOMStringAtom fontFaceName) noexcept
  {
    return AdoptRef(*new FontFaceSrcLocalValue(krys::move(fontFaceName)));
  }

  CSSOMString FontFaceSrcLocalValue::CustomCSSText(const SerialisationContext &) const noexcept
  {
    CSSOMString result {u8"local("};
    SerialiseString(CSSOMString(_fontFaceName.View()), result);
    result.push_back(u8')');
    return result;
  }

  bool FontFaceSrcLocalValue::Equals(const FontFaceSrcLocalValue &other) const noexcept
  {
    return _fontFaceName == other._fontFaceName;
  }

  FontFaceSrcResourceValue::FontFaceSrcResourceValue(CSSURL &&location, CSSOMString &&format,
                                                     List<FontTechnology> &&technologies) noexcept
      : Value(ValueType::FontFaceSrcResource), _location(Resolve(krys::move(location))),
        _format(krys::move(format)), _technologies(krys::move(technologies))
  {
  }

  Ref<FontFaceSrcResourceValue> FontFaceSrcResourceValue::Create(CSSURL location, CSSOMString format,
                                                                 List<FontTechnology> &&technologies) noexcept
  {
    return AdoptRef(
      *new FontFaceSrcResourceValue(krys::move(location), krys::move(format), krys::move(technologies)));
  }

  bool FontFaceSrcResourceValue::CustomTraverseSubresources(
    KRYS_NOESCAPE const Func<bool(const CachedResource &)> &) const noexcept
  {
    // FIXME: return `_cachedFont && handler(*_cachedFont)` once the resource loader is ported.
    return false;
  }

  bool FontFaceSrcResourceValue::CustomMayDependOnBaseUrl() const noexcept
  {
    return MayDependOnBaseURL(_location);
  }

  CSSOMString FontFaceSrcResourceValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    CSSOMString builder;

    Serialise(builder, context, _location);

    if (!_format.empty())
    {
      builder.append(u8" format(");
      SerialiseString(_format, builder);
      builder.push_back(u8')');
    }
    if (!_technologies.empty())
    {
      builder.append(u8" tech(");
      for (size_t i = 0uz; i < _technologies.size(); ++i)
      {
        if (i)
        {
          builder.append(u8", ");
        }
        builder.append(CssTextFromFontTech(_technologies[i]));
      }
      builder.push_back(u8')');
    }

    return builder;
  }

  bool FontFaceSrcResourceValue::Equals(const FontFaceSrcResourceValue &other) const noexcept
  {
    return _location == other._location && _format == other._format && _technologies == other._technologies;
  }
}
