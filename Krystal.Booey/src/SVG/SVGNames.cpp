#include "Krystal.Booey/SVG/SVGNames.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"

#define KRYS_DEFINE_SVG_ATTRIBUTE_NAME(Name, localName)                                                      \
  const dom::QualifiedName &SVGNames::Name() noexcept                                                        \
  {                                                                                                          \
    static NeverDestroyed<const dom::QualifiedName> name {dom::DOMStringAtom::Null(),                        \
                                                          dom::DOMStringAtom::Null(), localName};            \
    return name;                                                                                             \
  }

namespace krys::boo::svg
{
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(ClassAttr, u8"class")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(CxAttr, u8"cx")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(CyAttr, u8"cy")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(HeightAttr, u8"height")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(HrefAttr, u8"href")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(IdAttr, u8"id")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(MediaAttr, u8"media")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(PreserveAspectRatioAttr, u8"preserveAspectRatio")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(RAttr, u8"r")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(RequiredExtensionsAttr, u8"requiredExtensions")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(RxAttr, u8"rx")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(RyAttr, u8"ry")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(StyleAttr, u8"style")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(SystemLanguageAttr, u8"systemLanguage")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(TitleAttr, u8"title")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(TransformAttr, u8"transform")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(TypeAttr, u8"type")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(ViewBoxAttr, u8"viewBox")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(WidthAttr, u8"width")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(XAttr, u8"x")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(YAttr, u8"y")
  KRYS_DEFINE_SVG_ATTRIBUTE_NAME(ZoomAndPanAttr, u8"zoomAndPan")

  const dom::QualifiedName &XLinkNames::HrefAttr() noexcept
  {
    static NeverDestroyed<const dom::QualifiedName> name {dom::DOMStringAtom(infra::Namespaces::XLink),
                                                          dom::DOMStringAtom(infra::NamespacePrefixes::XLink),
                                                          dom::DOMStringAtom(u8"href")};
    return name;
  }
}

#undef KRYS_DEFINE_SVG_ATTRIBUTE_NAME
