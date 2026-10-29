#include "Krystal.Booey/Rendering/RenderFileUploadControl.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Layout/LayoutTestUtils.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/CSS/Selectors/UserAgentParts.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/FileAPI/FileList.hpp"
#include "Krystal.Booey/Graphics/DisplayLists/DisplayListItems.hpp"
#include "Krystal.Booey/Graphics/DisplayLists/DisplayListRecorderImpl.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include <format>
#include <string>

namespace krys::boo::layout::tests
{
  using style::tests::ResolverFixture;

  namespace
  {
    KRYS_NODISCARD RefPtr<FileList> Files(std::initializer_list<utf8_stringview> paths) noexcept
    {
      List<Ref<File>> files;
      for (auto path : paths)
      {
        files.push_back(File::Create(path));
      }
      return FileList::Create(krys::move(files));
    }

    KRYS_NODISCARD html::HTMLInputElement &ButtonOf(html::HTMLInputElement &input) noexcept
    {
      return Downcast<html::HTMLInputElement>(*input.UserAgentShadowRoot()->FirstChild());
    }

    KRYS_NODISCARD std::string Narrow(utf8_stringview text) noexcept
    {
      return std::string(text.begin(), text.end());
    }

    /// @brief The text runs @p fixture's view paints, as "glyphs@x,y", in order.
    KRYS_NODISCARD std::string PaintedRuns(ResolverFixture &fixture) noexcept
    {
      (void)fixture.Document->UpdateLayout();
      display_list::RecorderImpl recorder(FloatSize(800, 600));
      fixture.Frame->View()->Paint(recorder, IntRect(0, 0, 800, 600));
      std::string runs;
      for (const auto &item : recorder.TakeDisplayList()->Items())
      {
        if (auto *glyphs = std::get_if<display_list::DrawGlyphs>(&item))
        {
          runs += std::format("{}{}@{},{}", runs.empty() ? "" : " ", glyphs->Length(),
                              glyphs->LocalAnchor().X(), glyphs->LocalAnchor().Y());
        }
      }
      return runs;
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderFileUploadControl: the button and the file name",
                   "[Rendering][RenderFileUploadControl]")
  {
    ResolverFixture fixture {AhemDocument(u8"input{margin:0}", u8"<input type=file id=one><br>"
                                                               u8"<input type=file id=many multiple>")};
    auto &one = Downcast<html::HTMLInputElement>(fixture.ById(u8"one"));
    auto &many = Downcast<html::HTMLInputElement>(fixture.ById(u8"many"));
    // Room for 34 nominal characters (wider than the button and "no file selected" after it), with the
    // button in the shadow tree.
    CHECK(DumpWithIds(fixture) == u8R"(layer at (0,0) size 800x600
  RenderView at (0,0) size 800x600
layer at (0,0) size 800x38
  RenderBlock {HTML} at (0,0) size 800x38
    RenderBody {BODY} at (0,0) size 800x38
      RenderFileUploadControl {INPUT} at (0,0) size 340x19 id="one"
        RenderButton at (0,0) size 126x19 [color=#000000CC] [bgcolor=#C0C0C0] [border: (2px outset #C0C0C0)]
          RenderBlock (anonymous) at (8,4) size 110x10
            RenderTextFragment at (0,0) size 110x10
              text run at (0,0) width 110: "Choose File"
      RenderBR {BR} at (340,4) size 0x10
      RenderFileUploadControl {INPUT} at (0,19) size 340x19 id="many"
        RenderButton at (0,0) size 136x19 [color=#000000CC] [bgcolor=#C0C0C0] [border: (2px outset #C0C0C0)]
          RenderBlock (anonymous) at (8,4) size 120x10
            RenderTextFragment at (0,0) size 120x10
              text run at (0,0) width 120: "Choose Files"
)");
    auto &button = ButtonOf(one);
    CHECK(button.FormControlType() == u8"button");
    CHECK(button.UserAgentPart() == css::UserAgentParts::FileSelectorButton());
    auto &control = Downcast<RenderFileUploadControl>(*one.Renderer());
    CHECK(control.ButtonValue() == u8"Choose File");

    // The file name is drawn after the button, 4 pixels on, on its text's baseline, and before it.
    CHECK(control.FileTextValue() == u8"no file selected");
    CHECK(Downcast<RenderFileUploadControl>(*many.Renderer()).FileTextValue() == u8"no files selected");
    CHECK(PaintedRuns(fixture) == "16@130,12 11@8,12 17@140,31 12@8,31");

    SECTION("The files' names, or how many there are, cut to fit")
    {
      one.SetFiles(Files({u8"C:/drops/notes.txt"}));
      CHECK(control.FileTextValue() == u8"notes.txt");
      // The name has 210 pixels: the control's 340 less the button and the space after it.
      one.SetFiles(Files({u8"C:/drops/a-very-long-file-name-indeed.txt"}));
      CHECK(control.FileTextValue() == u8"a-very-lon…indeed.txt");
      many.SetFiles(Files({u8"C:/drops/a.txt", u8"C:/drops/b.txt"}));
      CHECK(Downcast<RenderFileUploadControl>(*many.Renderer()).FileTextValue() == u8"2 files");
      CHECK(PaintedRuns(fixture).starts_with("21@130,12"));
    }

    SECTION("A file name of both directions is drawn in visual order")
    {
      // "ab", then Hebrew shin and lamed: the Hebrew is drawn right to left, lamed first.
      one.SetFiles(Files({u8"C:/drops/abשל"}));
      (void)fixture.Document->UpdateLayout();
      display_list::RecorderImpl recorder(FloatSize(800, 600));
      fixture.Frame->View()->Paint(recorder, IntRect(0, 0, 800, 600));
      CHECK(boo::tests::DrawnText(*recorder.TakeDisplayList(), u8"של") == u8"לש");
    }

    SECTION("The button follows the input's multiple and disabled")
    {
      (void)one.SetAttribute(dom::DOMStringAtom(u8"multiple"), u8"");
      CHECK(button.Value() == u8"Choose Files");
      (void)one.ToggleAttribute(dom::DOMStringAtom(u8"multiple"), false);
      CHECK(button.Value() == u8"Choose File");

      CHECK_FALSE(button.IsDisabledFormControl());
      (void)one.SetAttribute(dom::DOMStringAtom(u8"disabled"), u8"");
      CHECK(button.IsDisabledFormControl());
      (void)one.ToggleAttribute(dom::DOMStringAtom(u8"disabled"), false);
      CHECK_FALSE(button.IsDisabledFormControl());
    }

    SECTION("A drag of files over the input makes its button active")
    {
      one.SetCanReceiveDroppedFiles(true);
      CHECK(button.Active());
      one.SetCanReceiveDroppedFiles(false);
      CHECK_FALSE(button.Active());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderFileUploadControl: widths",
                   "[Rendering][RenderFileUploadControl]")
  {
    // A fixed width is kept; a narrow one leaves no room for the name, which isn't drawn.
    ResolverFixture fixture {AhemDocument(u8"input{margin:0}",
                                          u8"<input type=file id=wide style=width:500px><br>"
                                          u8"<input type=file id=narrow style=width:100px>")};
    auto &wide = Downcast<html::HTMLInputElement>(fixture.ById(u8"wide"));
    auto &narrow = Downcast<html::HTMLInputElement>(fixture.ById(u8"narrow"));
    (void)fixture.Document->UpdateLayout();
    CHECK(wide.GetRenderBox()->Width() == 500);
    CHECK(narrow.GetRenderBox()->Width() == 100);
    CHECK(Downcast<RenderFileUploadControl>(*narrow.Renderer()).FileTextValue().empty());
  }
}
