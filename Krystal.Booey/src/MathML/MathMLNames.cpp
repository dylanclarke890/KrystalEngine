#include "Krystal.Booey/MathML/MathMLNames.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"

#define KRYS_DEFINE_MATHML_ATTRIBUTE_NAME(Name, localName)                                                   \
  const dom::QualifiedName &MathMLNames::Name() noexcept                                                     \
  {                                                                                                          \
    static NeverDestroyed<const dom::QualifiedName> name {dom::DOMStringAtom::Null(),                        \
                                                          dom::DOMStringAtom::Null(), localName};            \
    return name;                                                                                             \
  }

namespace krys::boo::mathml
{
  KRYS_DEFINE_MATHML_ATTRIBUTE_NAME(AccentAttr, u8"accent")
  KRYS_DEFINE_MATHML_ATTRIBUTE_NAME(AccentunderAttr, u8"accentunder")
  KRYS_DEFINE_MATHML_ATTRIBUTE_NAME(ColumnspanAttr, u8"columnspan")
  KRYS_DEFINE_MATHML_ATTRIBUTE_NAME(DepthAttr, u8"depth")
  KRYS_DEFINE_MATHML_ATTRIBUTE_NAME(DirAttr, u8"dir")
  KRYS_DEFINE_MATHML_ATTRIBUTE_NAME(DisplaystyleAttr, u8"displaystyle")
  KRYS_DEFINE_MATHML_ATTRIBUTE_NAME(FenceAttr, u8"fence")
  KRYS_DEFINE_MATHML_ATTRIBUTE_NAME(FormAttr, u8"form")
  KRYS_DEFINE_MATHML_ATTRIBUTE_NAME(HeightAttr, u8"height")
  KRYS_DEFINE_MATHML_ATTRIBUTE_NAME(LargeopAttr, u8"largeop")
  KRYS_DEFINE_MATHML_ATTRIBUTE_NAME(LinethicknessAttr, u8"linethickness")
  KRYS_DEFINE_MATHML_ATTRIBUTE_NAME(LspaceAttr, u8"lspace")
  KRYS_DEFINE_MATHML_ATTRIBUTE_NAME(MathbackgroundAttr, u8"mathbackground")
  KRYS_DEFINE_MATHML_ATTRIBUTE_NAME(MathcolorAttr, u8"mathcolor")
  KRYS_DEFINE_MATHML_ATTRIBUTE_NAME(MathsizeAttr, u8"mathsize")
  KRYS_DEFINE_MATHML_ATTRIBUTE_NAME(MathvariantAttr, u8"mathvariant")
  KRYS_DEFINE_MATHML_ATTRIBUTE_NAME(MaxsizeAttr, u8"maxsize")
  KRYS_DEFINE_MATHML_ATTRIBUTE_NAME(MinsizeAttr, u8"minsize")
  KRYS_DEFINE_MATHML_ATTRIBUTE_NAME(MovablelimitsAttr, u8"movablelimits")
  KRYS_DEFINE_MATHML_ATTRIBUTE_NAME(NotationAttr, u8"notation")
  KRYS_DEFINE_MATHML_ATTRIBUTE_NAME(RowspanAttr, u8"rowspan")
  KRYS_DEFINE_MATHML_ATTRIBUTE_NAME(RspaceAttr, u8"rspace")
  KRYS_DEFINE_MATHML_ATTRIBUTE_NAME(ScriptlevelAttr, u8"scriptlevel")
  KRYS_DEFINE_MATHML_ATTRIBUTE_NAME(SeparatorAttr, u8"separator")
  KRYS_DEFINE_MATHML_ATTRIBUTE_NAME(StretchyAttr, u8"stretchy")
  KRYS_DEFINE_MATHML_ATTRIBUTE_NAME(SymmetricAttr, u8"symmetric")
  KRYS_DEFINE_MATHML_ATTRIBUTE_NAME(VoffsetAttr, u8"voffset")
  KRYS_DEFINE_MATHML_ATTRIBUTE_NAME(WidthAttr, u8"width")
}

#undef KRYS_DEFINE_MATHML_ATTRIBUTE_NAME
