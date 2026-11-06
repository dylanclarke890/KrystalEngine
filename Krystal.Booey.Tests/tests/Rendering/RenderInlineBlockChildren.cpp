#include "catch2/catch_test_macros.hpp"
#include "catch2/generators/catch_generators.hpp"
#include "Krystal.Booey.Tests/Animation/AnimationTestUtils.hpp"
#include "Krystal.Booey.Tests/Layout/LayoutTestUtils.hpp"
#include "Krystal.Booey/DOM/Geometry/DOMRect.hpp"
#include "Krystal.Booey/DOM/Geometry/DOMRectList.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"

#include <format>

namespace krys::boo::tests
{
  namespace
  {
    constexpr utf8_stringview BaseStyle =
      u8"body{margin:0;font:20px/20px Ahem}#b{width:100px;height:30px;background:#0f0}";

    KRYS_NODISCARD std::string Format(const dom::DOMRectReadOnly &rect)
    {
      return std::format("{},{},{},{}", rect.X(), rect.Y(), rect.Width(), rect.Height());
    }

    /// @brief An element's boxes as the table in the test has them: `id: bounding | client client ...`.
    KRYS_NODISCARD std::string Boxes(AnimationPageFixture &fixture, const char8_t *id)
    {
      auto element = fixture.Document->GetElementById(dom::DOMStringView {id});
      if (!element)
      {
        return {};
      }
      std::string result = reinterpret_cast<const char *>(id);
      result += ": " + Format(*element->GetBoundingClientRect()) + " |";
      auto rects = element->GetClientRects();
      for (uint32 i = 0; i < rects->Length(); ++i)
      {
        result += " " + Format(*rects->Item(i));
      }
      return result;
    }
  }

  TEST_CASE("RenderInline: block children, as Chrome lays them out", "[Rendering][RenderInline]")
  {
    layout::tests::RegisterAhem();

    struct Case
    {
      const char8_t *Name;
      const char8_t *Style;
      const char8_t *Body;
      const char8_t *Boxes;
    };
    auto testCase = GENERATE(values<Case>({
      {u8"plain", u8"span{border:5px solid #00f;padding:0 3px;margin:0 7px;background:#ff0}",
       u8"<div id=o style='width:300px'>aa<span id=s>bb<div id=b></div>cc</span>dd</div>",
       u8"o: 0,0,300,70 | 0,0,300,70; s: 0,-5,300,80 | 47,-5,48,30 0,20,300,30 0,45,48,30; b: 0,20,100,30 | "
       u8"0,20,100,30"},
      {u8"block first", u8"span{border:5px solid #00f;padding:0 3px;margin:0 7px;background:#ff0}",
       u8"<div id=o style='width:300px'><span id=s><div id=b></div>cc</span></div>",
       u8"o: 0,0,300,70 | 0,0,300,70; s: 0,-5,300,80 | 7,-5,8,30 0,20,300,30 0,45,48,30; b: 0,20,100,30 | "
       u8"0,20,100,30"},
      {u8"block last", u8"span{border:5px solid #00f;padding:0 3px;margin:0 7px;background:#ff0}",
       u8"<div id=o style='width:300px'><span id=s>bb<div id=b></div></span></div>",
       u8"o: 0,0,300,70 | 0,0,300,70; s: 0,-5,300,80 | 7,-5,48,30 0,20,300,30 0,45,8,30; b: 0,20,100,30 | "
       u8"0,20,100,30"},
      {u8"block only", u8"span{border:5px solid #00f;padding:0 3px;margin:0 7px;background:#ff0}",
       u8"<div id=o style='width:300px'><span id=s><div id=b></div></span></div>",
       u8"o: 0,0,300,70 | 0,0,300,70; s: 0,-5,300,80 | 7,-5,8,30 0,20,300,30 0,45,8,30; b: 0,20,100,30 | "
       u8"0,20,100,30"},
      {u8"two blocks", u8"span{border:5px solid #00f;padding:0 3px;margin:0 7px;background:#ff0}",
       u8"<div id=o style='width:300px'><span id=s>bb<div id=b></div><div id=m "
       u8"style='height:10px'></div>cc</span></div>",
       u8"o: 0,0,300,80 | 0,0,300,80; s: 0,-5,300,90 | 7,-5,48,30 0,20,300,40 0,55,48,30; b: 0,20,100,30 | "
       u8"0,20,100,30; m: 0,50,300,10 | 0,50,300,10"},
      {u8"nested", u8"span{border:5px solid #00f;padding:0 3px;margin:0 7px;background:#ff0}",
       u8"<div id=o style='width:300px'><span id=s>bb<em id=e>xx<div id=b></div>yy</em>cc</span></div>",
       u8"o: 0,0,300,70 | 0,0,300,70; s: 0,-5,300,80 | 7,-5,88,30 0,20,300,30 0,45,88,30; e: 0,0,300,70 | "
       u8"55,0,40,20 0,20,300,30 0,50,40,20; b: 0,20,100,30 | 0,20,100,30"},
      {u8"margin block",
       u8"span{border:5px solid #00f;padding:0 3px;margin:0 7px;background:#ff0}#b{margin:10px 0}",
       u8"<div id=o style='width:300px'>aa<span id=s>bb<div id=b></div>cc</span>dd</div>",
       u8"o: 0,0,300,90 | 0,0,300,90; s: 0,-5,300,100 | 47,-5,48,30 0,30,300,30 0,65,48,30; b: 0,30,100,30 | "
       u8"0,30,100,30"},
      {u8"relative inline", u8"span{position:relative;left:10px;top:5px}",
       u8"<div id=o style='width:300px'>aa<span id=s>bb<div id=b></div>cc</span>dd</div>",
       u8"o: 0,0,300,70 | 0,0,300,70; s: 10,5,300,70 | 50,5,40,20 10,25,300,30 10,55,40,20; b: 10,25,100,30 "
       u8"| 10,25,100,30"},
      {u8"outline", u8"span{outline:2px solid red}",
       u8"<div id=o style='width:300px'>aa<span id=s>bb<div id=b></div>cc</span>dd</div>",
       u8"o: 0,0,300,70 | 0,0,300,70; s: 0,0,300,70 | 40,0,40,20 0,20,300,30 0,50,40,20; b: 0,20,100,30 | "
       u8"0,20,100,30"},
      {u8"background only", u8"span{background:#ff0}",
       u8"<div id=o style='width:300px'>aa<span id=s>bb<div id=b></div>cc</span>dd</div>",
       u8"o: 0,0,300,70 | 0,0,300,70; s: 0,0,300,70 | 40,0,40,20 0,20,300,30 0,50,40,20; b: 0,20,100,30 | "
       u8"0,20,100,30"},
      {u8"rtl", u8"span{border:5px solid #00f;padding:0 3px;margin:0 7px;background:#ff0}",
       u8"<div id=o style='width:300px;direction:rtl'>aa<span id=s>bb<div id=b></div>cc</span>dd</div>",
       u8"o: 0,0,300,70 | 0,0,300,70; s: 0,-5,300,80 | 245,-5,48,30 0,20,300,30 212,45,48,30; b: "
       u8"200,20,100,30 | 200,20,100,30"},
      {u8"abspos in relative",
       u8"span{position:relative}#a{position:absolute;left:1px;top:2px;width:5px;height:5px}",
       u8"<div id=o style='width:300px'>aa<span id=s>bb<div id=b></div>cc<div id=a></div></span>dd</div>",
       u8"o: 0,0,300,70 | 0,0,300,70; s: 0,0,300,70 | 40,0,40,20 0,20,300,30 0,50,40,20; b: 0,20,100,30 | "
       u8"0,20,100,30; a: 41,2,5,5 | 41,2,5,5"},
      {u8"abspos right bottom",
       u8"span{position:relative;padding:0 3px;border:5px solid "
       u8"#00f}#a{position:absolute;right:1px;bottom:2px;width:5px;height:5px}",
       u8"<div id=o style='width:300px'>aa<span id=s>bb<div id=b></div>cc<div id=a></div></span>dd</div>",
       u8"o: 0,0,300,70 | 0,0,300,70; s: 0,-5,300,80 | 40,-5,48,30 0,20,300,30 0,45,48,30; b: 0,20,100,30 | "
       u8"0,20,100,30; a: 39,63,5,5 | 39,63,5,5"},
      {u8"abspos percent", u8"span{position:relative}#a{position:absolute;left:0;top:0;width:50%;height:50%}",
       u8"<div id=o style='width:300px'>aa<span id=s>bb<div id=b></div>cc<div id=a></div></span>dd</div>",
       u8"o: 0,0,300,70 | 0,0,300,70; s: 0,0,300,70 | 40,0,40,20 0,20,300,30 0,50,40,20; b: 0,20,100,30 | "
       u8"0,20,100,30; a: 40,0,0,35 | 40,0,0,35"},
      {u8"abspos no block",
       u8"span{position:relative}#a{position:absolute;right:0;bottom:0;width:5px;height:5px}",
       u8"<div id=o style='width:300px'>aa<span id=s>bb<div id=a></div>cc</span>dd</div>",
       u8"o: 0,0,300,20 | 0,0,300,20; s: 40,0,80,20 | 40,0,80,20; a: 115,15,5,5 | 115,15,5,5"},
      {u8"abspos two lines",
       u8"span{position:relative}#a{position:absolute;left:1px;top:2px;width:50%;height:50%}",
       u8"<div id=o style='width:60px'>aaa <span id=s>bbbbb ccc<div id=a></div></span></div>",
       u8"o: 0,0,60,60 | 0,0,60,60; s: 0,20,100,40 | 0,20,100,20 0,40,60,20; a: 1,22,30,20 | 1,22,30,20"},
      {u8"abspos rtl",
       u8"span{position:relative}#a{position:absolute;right:1px;top:2px;width:5px;height:5px}",
       u8"<div id=o style='width:300px;direction:rtl'>aa<span id=s>bb<div id=b></div>cc<div "
       u8"id=a></div></span>dd</div>",
       u8"o: 0,0,300,70 | 0,0,300,70; s: 0,0,300,70 | 260,0,40,20 0,20,300,30 220,50,40,20; b: 200,20,100,30 "
       u8"| 200,20,100,30; a: 294,2,5,5 | 294,2,5,5"},
      {u8"abspos inline",
       u8"span{position:relative}i#a{position:absolute;right:0;bottom:0;width:5px;height:5px}",
       u8"<div id=o style='width:300px'>aa<span id=s>bb<div id=b></div>cc<i id=a></i></span>dd</div>",
       u8"o: 0,0,300,70 | 0,0,300,70; s: 0,0,300,70 | 40,0,40,20 0,20,300,30 0,50,40,20; b: 0,20,100,30 | "
       u8"0,20,100,30; a: 35,65,5,5 | 35,65,5,5"},
      {u8"abspos offset container",
       u8"body{margin:8px}#o{float:left;margin-left:20px}span{position:relative;left:8px;top:4px}#a{position:"
       u8"absolute;right:0;bottom:0;width:6px;height:6px}",
       u8"<div id=o style='width:300px;height:90px'>aa<span id=s>bb<div id=b></div>cc<i "
       u8"id=a></i></span>dd</div>",
       u8"o: 28,8,300,90 | 28,8,300,90; s: 36,12,300,70 | 76,12,40,20 36,32,300,30 36,62,40,20; b: "
       u8"36,32,100,30 | 36,32,100,30; a: 70,76,6,6 | 70,76,6,6"},
      {u8"blocks with margin between",
       u8"span{border:5px solid #00f;padding:0 3px;margin:0 7px;background:#ff0}#m{margin-top:5px}",
       u8"<div id=o style='width:300px'><span id=s>bb<div id=b></div><div id=m "
       u8"style='height:10px'></div>cc</span></div>",
       u8"o: 0,0,300,85 | 0,0,300,85; s: 0,-5,300,95 | 7,-5,48,30 0,20,300,45 0,60,48,30; b: 0,20,100,30 | "
       u8"0,20,100,30; m: 0,55,300,10 | 0,55,300,10"},
      {u8"blocks with collapsing margins",
       u8"span{border:5px solid #00f;padding:0 3px;margin:0 "
       u8"7px;background:#ff0}#b{margin-bottom:8px}#m{margin-top:5px}",
       u8"<div id=o style='width:300px'><span id=s>bb<div id=b></div><div id=m "
       u8"style='height:10px'></div>cc</span></div>",
       u8"o: 0,0,300,88 | 0,0,300,88; s: 0,-5,300,98 | 7,-5,48,30 0,20,300,48 0,63,48,30; b: 0,20,100,30 | "
       u8"0,20,100,30; m: 0,58,300,10 | 0,58,300,10"},
      {u8"three blocks", u8"",
       u8"<div id=o style='width:300px'><span id=s>bb<div id=b></div><div id=m "
       u8"style='height:10px'></div><div style='height:10px'></div>cc</span></div>",
       u8"o: 0,0,300,90 | 0,0,300,90; s: 0,0,300,90 | 0,0,40,20 0,20,300,50 0,70,40,20; b: 0,20,100,30 | "
       u8"0,20,100,30; m: 0,50,300,10 | 0,50,300,10"},
      {u8"blocks in nested inline", u8"",
       u8"<div id=o style='width:300px'><span id=s><em id=e><div id=b></div></em><div id=m "
       u8"style='height:10px'></div></span></div>",
       u8"o: 0,0,300,40 | 0,0,300,40; s: 0,0,300,40 | 0,0,0,0 0,0,300,30 0,30,0,0 0,30,300,10 0,40,0,0; e: "
       u8"0,0,300,30 | 0,0,0,0 0,0,300,30 0,30,0,0; b: 0,0,100,30 | 0,0,100,30; m: 0,30,300,10 | "
       u8"0,30,300,10"},
      {u8"margin block last",
       u8"span{border:5px solid #00f;padding:0 3px;margin:0 7px;background:#ff0}#b{margin:10px 0}",
       u8"<div id=o style='width:300px'><span id=s>bb<div id=b></div></span></div>",
       u8"o: 0,0,300,90 | 0,0,300,90; s: 0,-5,300,100 | 7,-5,48,30 0,30,300,30 0,65,8,30; b: 0,30,100,30 | "
       u8"0,30,100,30"},
      {u8"margin block last plain", u8"#b{margin:10px 0}",
       u8"<div id=o style='width:300px'><span id=s>bb<div id=b></div></span></div>",
       u8"o: 0,0,300,60 | 0,0,300,60; s: 0,0,300,60 | 0,0,40,20 0,30,300,30 0,70,0,0; b: 0,30,100,30 | "
       u8"0,30,100,30"},
      {u8"margin block first plain", u8"#b{margin:10px 0}",
       u8"<div id=o style='width:300px'><span id=s><div id=b></div>cc</span></div>",
       u8"o: 0,10,300,60 | 0,10,300,60; s: 0,10,300,60 | 0,10,0,0 0,10,300,30 0,50,40,20; b: 0,10,100,30 | "
       u8"0,10,100,30"},
      {u8"margin only padded o", u8"#o{padding:5px 0}#b{margin:10px 0}",
       u8"<div id=o style='width:300px'><span id=s><div id=b></div></span></div>",
       u8"o: 0,0,300,60 | 0,0,300,60; s: 0,15,300,30 | 0,5,0,0 0,15,300,30 0,55,0,0; b: 0,15,100,30 | "
       u8"0,15,100,30"},
      {u8"float wider",
       u8"span{border:5px solid #00f;padding:0 3px;margin:0 "
       u8"7px;background:#ff0}#f{float:left;width:50px;height:80px}",
       u8"<div id=o style='width:300px'><div id=f></div>aa<span id=s>bb<div id=b "
       u8"style='width:auto'></div>cc</span>dd</div>",
       u8"o: 0,0,300,70 | 0,0,300,70; s: 0,-5,300,80 | 97,-5,48,30 0,20,300,30 50,45,48,30; b: 0,20,300,30 | "
       u8"0,20,300,30; f: 0,0,50,80 | 0,0,50,80"},
      {u8"float right",
       u8"span{border:5px solid #00f;padding:0 3px;margin:0 "
       u8"7px;background:#ff0}#f{float:right;width:20px;height:50px}",
       u8"<div id=o style='width:300px'><div id=f></div>aa<span id=s>bb<div id=b></div>cc</span>dd</div>",
       u8"o: 0,0,300,70 | 0,0,300,70; s: 0,-5,300,80 | 47,-5,48,30 0,20,300,30 0,45,48,30; b: 0,20,100,30 | "
       u8"0,20,100,30; f: 280,0,20,50 | 280,0,20,50"},
      {u8"float rtl",
       u8"span{border:5px solid #00f;padding:0 3px;margin:0 "
       u8"7px;background:#ff0}#f{float:right;width:20px;height:50px}",
       u8"<div id=o style='width:300px;direction:rtl'><div id=f></div>aa<span id=s>bb<div "
       u8"id=b></div>cc</span>dd</div>",
       u8"o: 0,0,300,70 | 0,0,300,70; s: 0,-5,300,80 | 225,-5,48,30 0,20,300,30 212,45,48,30; b: "
       u8"200,20,100,30 | 200,20,100,30; f: 280,0,20,50 | 280,0,20,50"},
      {u8"margin block first",
       u8"span{border:5px solid #00f;padding:0 3px;margin:0 7px;background:#ff0}#b{margin:10px 0}",
       u8"<div id=o style='width:300px'><span id=s><div id=b></div>cc</span></div>",
       u8"o: 0,0,300,90 | 0,0,300,90; s: 0,-5,300,100 | 7,-5,8,30 0,30,300,30 0,65,48,30; b: 0,30,100,30 | "
       u8"0,30,100,30"},
      {u8"margin block only",
       u8"span{border:5px solid #00f;padding:0 3px;margin:0 7px;background:#ff0}#b{margin:10px 0}",
       u8"<div id=o style='width:300px'><span id=s><div id=b></div></span></div>",
       u8"o: 0,0,300,90 | 0,0,300,90; s: 0,-5,300,100 | 7,-5,8,30 0,30,300,30 0,65,8,30; b: 0,30,100,30 | "
       u8"0,30,100,30"},
      {u8"empty span", u8"", u8"<div id=o style='width:300px'><span id=s></span></div>",
       u8"o: 0,0,300,0 | 0,0,300,0; s: 0,0,0,0 | 0,0,0,0"},
      {u8"empty nested span", u8"", u8"<div id=o style='width:300px'><span id=s><em id=e></em></span></div>",
       u8"o: 0,0,300,0 | 0,0,300,0; s: 0,0,0,0 | 0,0,0,0; e: 0,0,0,0 | 0,0,0,0"},
      {u8"empty span, text after", u8"", u8"<div id=o style='width:300px'><span id=s></span>aa</div>",
       u8"o: 0,0,300,20 | 0,0,300,20; s: 0,0,0,20 | 0,0,0,20"},
      {u8"empty span, line break", u8"", u8"<div id=o style='width:300px'><span id=s></span><br>aa</div>",
       u8"o: 0,0,300,40 | 0,0,300,40; s: 0,0,0,20 | 0,0,0,20"},
      {u8"empty nested span, block after", u8"",
       u8"<div id=o style='width:300px'><span id=s><em id=e></em></span><div id=b></div></div>",
       u8"o: 0,0,300,30 | 0,0,300,30; s: 0,0,0,0 | 0,0,0,0; e: 0,0,0,0 | 0,0,0,0; b: 0,0,100,30 | "
       u8"0,0,100,30"},
      {u8"percent width block",
       u8"span{border:5px solid #00f;padding:0 3px;margin:0 7px;background:#ff0}#b{width:50%}",
       u8"<div id=o style='width:300px'>aa<span id=s>bb<div id=b></div>cc</span>dd</div>",
       u8"o: 0,0,300,70 | 0,0,300,70; s: 0,-5,300,80 | 47,-5,48,30 0,20,300,30 0,45,48,30; b: 0,20,150,30 | "
       u8"0,20,150,30"},
      {u8"float sibling",
       u8"span{border:5px solid #00f;padding:0 3px;margin:0 "
       u8"7px;background:#ff0}#f{float:left;width:20px;height:50px}",
       u8"<div id=o style='width:300px'><div id=f></div>aa<span id=s>bb<div id=b></div>cc</span>dd</div>",
       u8"o: 0,0,300,70 | 0,0,300,70; s: 0,-5,300,80 | 67,-5,48,30 0,20,300,30 0,45,48,30; b: 0,20,100,30 | "
       u8"0,20,100,30; f: 0,0,20,50 | 0,0,20,50"},
      {u8"anon after", u8"span{border:5px solid #00f;padding:0 3px;margin:0 7px;background:#ff0}",
       u8"<div id=o style='width:300px'><span id=s>bb<div id=b></div></span>dd</div>",
       u8"o: 0,0,300,70 | 0,0,300,70; s: 0,-5,300,80 | 7,-5,48,30 0,20,300,30 0,45,8,30; b: 0,20,100,30 | "
       u8"0,20,100,30"},
      {u8"shrink", u8"span{border:5px solid #00f;padding:0 3px;margin:0 7px;background:#ff0}#o{float:left}",
       u8"<div id=o>aa<span id=s>bb<div id=b></div>cc</span>dd</div>",
       u8"o: 0,0,100,70 | 0,0,100,70; s: 0,-5,100,80 | 47,-5,48,30 0,20,100,30 0,45,48,30; b: 0,20,100,30 | "
       u8"0,20,100,30"},
      {u8"margin collapse", u8"#o{margin:0}#b{margin-top:10px;margin-bottom:10px}",
       u8"<div id=o style='width:300px'><span id=s><div id=b></div></span></div>",
       u8"o: 0,10,300,30 | 0,10,300,30; s: 0,10,300,30 | 0,10,0,0 0,10,300,30 0,50,0,0; b: 0,10,100,30 | "
       u8"0,10,100,30"},
    }));

    AnimationPageFixture fixture {u8"<!DOCTYPE html><style>" + utf8_string(BaseStyle) + testCase.Style
                                  + u8"</style>" + testCase.Body};
    (void)fixture.Document->UpdateLayout();

    std::string boxes;
    for (auto *id : {u8"o", u8"s", u8"e", u8"b", u8"m", u8"a", u8"f"})
    {
      auto element = Boxes(fixture, id);
      if (!element.empty())
      {
        boxes += (boxes.empty() ? "" : "; ") + element;
      }
    }
    INFO(reinterpret_cast<const char *>(testCase.Name));
    CHECK(boxes == reinterpret_cast<const char *>(testCase.Boxes));
  }

  TEST_CASE("RenderInline: an out-of-flow child follows the inline box", "[Rendering][RenderInline]")
  {
    layout::tests::RegisterAhem();

    constexpr utf8_stringview AbsoluteAtEnd =
      u8"span{position:relative}#a{position:absolute;right:0;bottom:0;width:5px;height:5px}";
    auto rects = [](AnimationPageFixture &fixture)
    {
      (void)fixture.Document->UpdateLayout();
      return Boxes(fixture, u8"s") + "; " + Boxes(fixture, u8"a");
    };
    auto page = [](utf8_stringview style, utf8_stringview body)
    {
      return u8"<!DOCTYPE html><style>" + utf8_string(BaseStyle) + utf8_string(style) + u8"</style>"
             + utf8_string(body);
    };

    SECTION("where it's hit, not just where it's said to be")
    {
      AnimationPageFixture fixture {
        page(AbsoluteAtEnd, u8"<div style='width:300px'>aa<span id=s>bb<i id=a></i></span>dd</div>")};
      (void)fixture.Document->UpdateLayout();
      CHECK(rects(fixture) == "s: 40,0,40,20 | 40,0,40,20; a: 75,15,5,5 | 75,15,5,5");
      CHECK(fixture.Document->ElementFromPoint(77, 17).get() == &fixture.ById(u8"a"));
    }

    SECTION("when its text grows")
    {
      AnimationPageFixture fixture {page(
        AbsoluteAtEnd, u8"<div style='width:300px'>aa<span id=s><i id=t>bb</i><i id=a></i></span>dd</div>")};
      (void)fixture.Document->UpdateLayout();
      REQUIRE_FALSE(fixture.ById(u8"t").TextContent(dom::DOMString {u8"bbbb"}).HasException());
      CHECK(rects(fixture) == "s: 40,0,80,20 | 40,0,80,20; a: 115,15,5,5 | 115,15,5,5");
    }

    SECTION("when the text before it grows")
    {
      AnimationPageFixture fixture {
        page(u8"span{position:relative}#a{position:absolute;left:0;top:0;width:5px;height:5px}",
             u8"<div style='width:300px'><i id=t>aa</i><span id=s>bb<i id=a></i></span>dd</div>")};
      (void)fixture.Document->UpdateLayout();
      REQUIRE_FALSE(fixture.ById(u8"t").TextContent(dom::DOMString {u8"aaaa"}).HasException());
      CHECK(rects(fixture) == "s: 80,0,40,20 | 80,0,40,20; a: 80,0,5,5 | 80,0,5,5");
    }

    SECTION("when its text grows after a block")
    {
      AnimationPageFixture fixture {
        page(AbsoluteAtEnd, u8"<div style='width:300px'>aa<span id=s>bb<div id=b></div><i id=t>cc</i><i "
                            u8"id=a></i></span>dd</div>")};
      (void)fixture.Document->UpdateLayout();
      REQUIRE_FALSE(fixture.ById(u8"t").TextContent(dom::DOMString {u8"cccc"}).HasException());
      CHECK(rects(fixture) == "s: 0,0,300,70 | 40,0,40,20 0,20,300,30 0,50,80,20; a: 75,65,5,5 | 75,65,5,5");
    }

    SECTION("when a block in it grows")
    {
      AnimationPageFixture fixture {
        page(AbsoluteAtEnd,
             u8"<div style='width:300px'>aa<span id=s>bb<div id=b></div>cc<i id=a></i></span>dd</div>")};
      (void)fixture.Document->UpdateLayout();
      fixture.SetStyle(u8"b", u8"height:60px");
      CHECK(rects(fixture) == "s: 0,0,300,100 | 40,0,40,20 0,20,300,60 0,80,40,20; a: 35,95,5,5 | 35,95,5,5");
    }
  }
}
