#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Editing/Editor.hpp"
#include "Krystal.Booey/Editing/FrameSelection.hpp"
#include "Krystal.Booey/Editing/VisiblePosition.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/HTML/HTMLTextFormControlElement.hpp"
#include "Krystal.Booey/Page/DOMSelection.hpp"
#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Core/Text/CodePointIterator.hpp"
#include "Krystal.Gfx.Tests/Golden.hpp"
#include "Krystal.UI.Tests/ViewTestUtils.hpp"
#include "Krystal.UI/View.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <format>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace krys::ui::tests
{
  namespace
  {
    /// @brief How far the caret may be from Chrome's.
    constexpr double CaretTolerance = 1.0;

    /// @brief The node at @p path: an element's id, then the indices of children below it.
    KRYS_NODISCARD boo::dom::Node *NodeAt(boo::dom::Document &document, const std::string &path)
    {
      std::istringstream stream(path);
      std::string part;
      std::getline(stream, part, '/');
      boo::dom::Node *node = document.GetElementById(boo::dom::DOMString(part.begin(), part.end())).get();
      while (node && std::getline(stream, part, '/'))
      {
        auto index = std::stoi(part);
        node = node->FirstChild();
        for (; node && index > 0; --index)
        {
          node = node->NextSibling();
        }
      }
      return node;
    }

    /// @brief @p node's path, as `NodeAt` reads it.
    KRYS_NODISCARD std::string PathOf(const boo::dom::Node &node)
    {
      std::string path;
      const boo::dom::Node *current = &node;
      while (current)
      {
        if (auto *element = DynamicDowncast<boo::dom::Element>(*current))
        {
          if (auto id = element->GetAttribute(u8"id"))
          {
            return std::string(id->begin(), id->end()) + path;
          }
        }
        path = "/" + std::to_string(current->ComputeNodeIndex()) + path;
        current = current->ParentNode();
      }
      return "?" + path;
    }

    /// @brief The length of @p text in UTF-16 code units, as Chrome's DOM counts.
    KRYS_NODISCARD uint32 UTF16Length(utf8_stringview text)
    {
      uint32 units = 0;
      for (text::CodePointIterator it {text}; !it.AtEnd(); ++it)
      {
        units += *it > 0xFFFF ? 2u : 1u;
      }
      return units;
    }

    /// @brief A text node's offset in UTF-16 code units (as Chrome's DOM counts) from its offset in bytes;
    /// other nodes' offsets count children either way.
    KRYS_NODISCARD uint32 ToUTF16Offset(const boo::dom::Node &node, uint32 offset)
    {
      auto *text = DynamicDowncast<boo::dom::Text>(node);
      if (!text)
      {
        return offset;
      }
      return UTF16Length(utf8_stringview(text->Data()).substr(0, offset));
    }

    /// @brief The inverse of `ToUTF16Offset`.
    KRYS_NODISCARD uint32 FromUTF16Offset(const boo::dom::Node &node, uint32 offset)
    {
      auto *text = DynamicDowncast<boo::dom::Text>(node);
      if (!text)
      {
        return offset;
      }
      utf8_stringview data = text->Data();
      uint32 units = 0;
      text::CodePointIterator it {data};
      for (; !it.AtEnd() && units < offset; ++it)
      {
        units += *it > 0xFFFF ? 2u : 1u;
      }
      return static_cast<uint32>(it.CodeUnitsSince(data.data()));
    }

    /// @brief Whether @p chrome's endpoint (a path and offset) and Booey's are the same visible position. A
    /// range can be held by different but equivalent endpoints: extending forward, Chrome moves its anchor to
    /// the position's canonical form, as the start of the next text node, where Booey keeps the original.
    KRYS_NODISCARD bool AreEquivalent(boo::dom::Document &document, const std::string &chrome,
                                      const std::string &booey)
    {
      auto endpoint = [&](const std::string &text) -> boo::VisiblePosition
      {
        auto colon = text.rfind(':');
        auto *node = NodeAt(document, text.substr(0, colon));
        if (!node)
        {
          return {};
        }
        auto offset = FromUTF16Offset(*node, static_cast<uint32>(std::stoul(text.substr(colon + 1))));
        return boo::VisiblePosition(boo::dom::MakeDeprecatedLegacyPosition(ShareRefPtr(node), offset));
      };
      auto a = endpoint(chrome);
      return !a.IsNull() && a == endpoint(booey);
    }

    struct EditingState
    {
      std::string Anchor;
      std::string Focus;
      std::string CaretX;
    };

    KRYS_NODISCARD EditingState StateOf(boo::dom::Document &document)
    {
      auto &selection = *document.GetSelection();
      EditingState state;
      auto anchor = selection.AnchorNode();
      auto focus = selection.FocusNode();
      if (!anchor || !focus)
      {
        return {"none", "none", "-"};
      }
      state.Anchor = PathOf(*anchor) + ":" + std::to_string(ToUTF16Offset(*anchor, selection.AnchorOffset()));
      state.Focus = PathOf(*focus) + ":" + std::to_string(ToUTF16Offset(*focus, selection.FocusOffset()));
      state.CaretX = "-";
      if (selection.IsCollapsed())
      {
        state.CaretX =
          std::format("{:.2f}", static_cast<double>(document.Selection().AbsoluteCaretBounds().X()));
      }
      return state;
    }

    using Rect = std::array<double, 4>;

    /// @brief Rects that touch on a line, merged: Chrome and Booey can split a selection's text differently.
    KRYS_NODISCARD std::vector<Rect> MergeTouching(std::vector<Rect> rects)
    {
      std::ranges::sort(rects, [](const Rect &a, const Rect &b)
                        { return std::tie(a[1], a[0]) < std::tie(b[1], b[0]); });
      std::vector<Rect> merged;
      for (auto &rect : rects)
      {
        if (!merged.empty())
        {
          auto &last = merged.back();
          if (std::abs(last[1] - rect[1]) <= CaretTolerance && std::abs(last[3] - rect[3]) <= CaretTolerance
              && rect[0] <= last[0] + last[2] + CaretTolerance)
          {
            last[2] = std::max(last[0] + last[2], rect[0] + rect[2]) - last[0];
            continue;
          }
        }
        merged.push_back(rect);
      }
      return merged;
    }

    /// @brief Rects written as "x,y,w,h" joined by ";", or "-" for none.
    KRYS_NODISCARD std::vector<Rect> ParseRects(const std::string &text)
    {
      std::vector<Rect> rects;
      if (text == "-")
      {
        return rects;
      }
      std::istringstream stream(text);
      for (std::string part; std::getline(stream, part, ';');)
      {
        Rect rect {};
        std::istringstream values(part);
        std::string value;
        for (auto &component : rect)
        {
          std::getline(values, value, ',');
          component = std::stod(value);
        }
        rects.push_back(rect);
      }
      return rects;
    }

    KRYS_NODISCARD bool RectsMatch(const std::string &chrome, const std::string &booey)
    {
      auto a = MergeTouching(ParseRects(chrome));
      auto b = MergeTouching(ParseRects(booey));
      if (a.size() != b.size())
      {
        return false;
      }
      // Each edge within a pixel: Booey snaps a partly selected box's rect to pixels, and Chrome doesn't.
      for (size_t i = 0; i < a.size(); ++i)
      {
        Rect edgesA {a[i][0], a[i][1], a[i][0] + a[i][2], a[i][1] + a[i][3]};
        Rect edgesB {b[i][0], b[i][1], b[i][0] + b[i][2], b[i][1] + b[i][3]};
        for (size_t j = 0; j < 4; ++j)
        {
          if (std::abs(edgesA[j] - edgesB[j]) > CaretTolerance)
          {
            return false;
          }
        }
      }
      return true;
    }

    /// @brief The selection's client rects, as the scene's script writes them.
    KRYS_NODISCARD std::string SelectionRects(boo::dom::Document &document)
    {
      auto &selection = *document.GetSelection();
      if (!selection.RangeCount() || selection.IsCollapsed())
      {
        return "-";
      }
      auto range = selection.GetRangeAt(0).ReleaseValue();
      auto list = range->GetClientRects();
      std::string text;
      for (uint32 i = 0; i < list->Length(); ++i)
      {
        auto *rect = list->Item(i);
        text += std::format("{}{:.2f},{:.2f},{:.2f},{:.2f}", text.empty() ? "" : ";", rect->X(), rect->Y(),
                            rect->Width(), rect->Height());
      }
      return text.empty() ? "-" : text;
    }

    /// @brief The text of the paragraph holding the selection's focus.
    KRYS_NODISCARD std::string FocusParagraphText(boo::dom::Document &document)
    {
      for (boo::dom::Node *node = document.GetSelection()->FocusNode().get(); node; node = node->ParentNode())
      {
        if (auto *element = DynamicDowncast<boo::dom::Element>(*node);
            element && element->LocalName() == u8"p")
        {
          auto text = element->TextContent().value_or(boo::dom::DOMString {});
          return std::string(text.begin(), text.end());
        }
      }
      return {};
    }

    /// @brief Sends @p scene's actions to a view as real input, and checks the state after each against
    /// Chrome's.
    void CheckInputAgainstChrome(const std::string &scene)
    {
      std::ifstream file(gfx::tests::GoldenDirectory() / (scene + ".chrome-input.txt"));
      REQUIRE(file);

      static const auto fonts = View::AddFontDirectory(std::filesystem::path {KRYS_UI_TESTS_FONTS_DIR});
      (void)fonts;
      View view({360, 200});
      REQUIRE(LoadAndSettle(view, gfx::tests::GoldenDirectory() / "scenes" / (scene + ".html")));
      auto &document = view.Document();

      auto point = [](const std::string &x, const std::string &y)
      {
        return boo::IntPoint(static_cast<int32>(std::stod(x)), static_cast<int32>(std::stod(y)));
      };
      auto click = [&](boo::IntPoint at, pal::KeyModifiers modifiers)
      {
        view.MouseMoved(at, modifiers);
        view.MousePressed(at, pal::MouseButton::Left, modifiers);
        view.MouseReleased(at, pal::MouseButton::Left, modifiers);
      };

      std::string mismatches;
      uint32 count = 0;
      uint32 lineNumber = 0;
      for (std::string line; std::getline(file, line);)
      {
        ++lineNumber;
        auto arrow = line.find(" => ");
        if (arrow == std::string::npos)
        {
          continue;
        }
        auto action = line.substr(0, arrow);
        auto space = action.find(' ');
        auto verb = action.substr(0, space);
        auto rest = space == std::string::npos ? std::string {} : action.substr(space + 1);
        std::istringstream arguments(rest);
        std::vector<std::string> args;
        for (std::string argument; arguments >> argument;)
        {
          args.push_back(argument);
        }

        if (verb == "click" || verb == "shiftclick")
        {
          pal::KeyModifiers modifiers;
          if (verb == "shiftclick")
          {
            modifiers.Add(pal::KeyModifier::Shift);
          }
          click(point(args[0], args[1]), modifiers);
        }
        else if (verb == "dblclick")
        {
          click(point(args[0], args[1]), {});
          click(point(args[0], args[1]), {});
        }
        else if (verb == "drag")
        {
          auto from = point(args[0], args[1]);
          auto to = point(args[2], args[3]);
          view.MouseMoved(from);
          view.MousePressed(from, pal::MouseButton::Left);
          for (int32 step = 1; step <= 4; ++step)
          {
            view.MouseMoved(
              {from.X() + (to.X() - from.X()) * step / 4, from.Y() + (to.Y() - from.Y()) * step / 4});
          }
          view.MouseReleased(to, pal::MouseButton::Left);
        }
        else if (verb == "type")
        {
          (void)view.InsertText(utf8_string(rest.begin(), rest.end()));
        }
        else if (verb == "key")
        {
          auto key = args[0] == "Backspace"    ? pal::Key::Del
                     : args[0] == "Delete"     ? pal::Key::ForwardDelete
                     : args[0] == "ArrowLeft"  ? pal::Key::LeftArrow
                     : args[0] == "ArrowRight" ? pal::Key::RightArrow
                     : args[0] == "Home"       ? pal::Key::Home
                                               : pal::Key::End;
          pal::KeyModifiers modifiers;
          if (args.size() > 1 && args[1] == "shift")
          {
            modifiers.Add(pal::KeyModifier::Shift);
          }
          (void)view.KeyPressed(key, modifiers);
          (void)view.KeyReleased(key, modifiers);
        }
        view.UpdateRendering();
        boo::html::EventLoop::Current().RunUntilIdle();

        std::istringstream expected(line.substr(arrow + 4));
        EditingState chrome;
        std::string chromeRects;
        expected >> chrome.Anchor >> chrome.Focus >> chrome.CaretX >> chromeRects;
        std::string chromeText;
        std::getline(expected, chromeText);
        chromeText = chromeText.substr(chromeText.find("text=") + 5);

        EditingState actual;
        std::string actualRects = "-";
        std::string actualText;
        if (auto *field = DynamicDowncast<boo::html::HTMLTextFormControlElement>(document.FocusedElement()))
        {
          // A field's selection, by its offsets into its value.
          auto id = field->GetAttribute(u8"id").value_or(boo::dom::DOMString {});
          auto value = field->Value();
          utf8_stringview text(value);
          auto offset = [&](uint32 bytes)
          {
            return std::to_string(UTF16Length(text.substr(0, bytes)));
          };
          actual.Anchor = std::string(id.begin(), id.end()) + ":" + offset(field->SelectionStart());
          actual.Focus = std::string(id.begin(), id.end()) + ":" + offset(field->SelectionEnd());
          actual.CaretX = "-";
          for (auto c : text)
          {
            actualText += c == '\n' ? std::string("\\n") : std::string(1, static_cast<char>(c));
          }
        }
        else
        {
          actual = StateOf(document);
          actualRects = SelectionRects(document);
          actualText = FocusParagraphText(document);
        }
        bool caretMatches =
          chrome.CaretX == actual.CaretX
          || (chrome.CaretX != "-" && actual.CaretX != "-"
              && std::abs(std::stod(chrome.CaretX) - std::stod(actual.CaretX)) <= CaretTolerance);
        if (chrome.Anchor != actual.Anchor || chrome.Focus != actual.Focus || !caretMatches
            || !RectsMatch(chromeRects, actualRects) || chromeText != actualText)
        {
          ++count;
          mismatches +=
            std::format("{}: {} => Chrome {} {} {} {} text={}\n   Booey {} {} {} {} text={}\n", lineNumber,
                        action, chrome.Anchor, chrome.Focus, chrome.CaretX, chromeRects, chromeText,
                        actual.Anchor, actual.Focus, actual.CaretX, actualRects, actualText);
        }
      }
      INFO(count << " actions differ from Chrome's:\n" << mismatches);
      CHECK(count == 0u);
    }

  }

  TEST_CASE("Bidi editing: the caret moves through mixed text as Chrome's does", "[UI][Editing][Bidi]")
  {
    std::ifstream file(gfx::tests::GoldenDirectory() / "bidi-editing.chrome-editing.txt");
    REQUIRE(file);

    static const auto fonts = View::AddFontDirectory(std::filesystem::path {KRYS_UI_TESTS_FONTS_DIR});
    (void)fonts;
    View view({360, 200});
    REQUIRE(LoadAndSettle(view, gfx::tests::GoldenDirectory() / "scenes" / "bidi-editing.html"));
    auto &document = view.Document();
    auto &selection = *document.GetSelection();

    std::string mismatches;
    uint32 count = 0;
    uint32 lineNumber = 0;
    for (std::string line; std::getline(file, line);)
    {
      ++lineNumber;
      auto arrow = line.find(" => ");
      if (arrow == std::string::npos)
      {
        continue;
      }
      std::istringstream operation(line.substr(0, arrow));
      std::istringstream expected(line.substr(arrow + 4));
      std::string verb;
      operation >> verb;
      if (verb == "set")
      {
        std::string path;
        uint32 offset = 0;
        operation >> path >> offset;
        auto *node = NodeAt(document, path);
        REQUIRE(node);
        REQUIRE_FALSE(selection.Collapse(node, FromUTF16Offset(*node, offset)).HasException());
      }
      else
      {
        std::string alter, direction, granularity;
        operation >> alter >> direction >> granularity;
        selection.Modify(boo::dom::DOMString(alter.begin(), alter.end()),
                         boo::dom::DOMString(direction.begin(), direction.end()),
                         boo::dom::DOMString(granularity.begin(), granularity.end()));
      }

      EditingState chrome;
      expected >> chrome.Anchor >> chrome.Focus >> chrome.CaretX;
      auto actual = StateOf(document);
      bool caretMatches =
        chrome.CaretX == actual.CaretX
        || (chrome.CaretX != "-" && actual.CaretX != "-"
            && std::abs(std::stod(chrome.CaretX) - std::stod(actual.CaretX)) <= CaretTolerance);
      auto endpointMatches = [&](const std::string &a, const std::string &b)
      {
        return a == b || (chrome.CaretX == "-" && AreEquivalent(document, a, b));
      };
      if (!endpointMatches(chrome.Anchor, actual.Anchor) || !endpointMatches(chrome.Focus, actual.Focus)
          || !caretMatches)
      {
        ++count;
        mismatches +=
          std::format("{}: {} => Chrome {} {} {}, Booey {} {} {}\n", lineNumber, line.substr(0, arrow),
                      chrome.Anchor, chrome.Focus, chrome.CaretX, actual.Anchor, actual.Focus, actual.CaretX);
      }
    }
    INFO(count << " steps differ from Chrome's:\n" << mismatches);
    CHECK(count == 0u);
  }

  TEST_CASE("Bidi editing: clicks, drags and typing in mixed text do what Chrome's do", "[UI][Editing][Bidi]")
  {
    CheckInputAgainstChrome("bidi-input");
  }

  TEST_CASE("Bidi editing: clicks, keys and typing in text fields do what Chrome's do", "[UI][Editing][Bidi]")
  {
    CheckInputAgainstChrome("bidi-fields");
  }

  TEST_CASE("Bidi editing: Ctrl with a Shift key sets the direction of the text being edited",
            "[UI][Editing][Bidi]")
  {
    View view({300, 200});
    view.LoadHTML(u8"<!DOCTYPE html><style>body { margin: 0; font: 16px/20px sans-serif }"
                  u8" input, p { display: block; margin: 0; width: 200px; height: 20px }</style>"
                  u8"<input id=f value=abc><p id=p contenteditable>hello world</p>");
    WaitForLoad(view);
    view.UpdateRendering();
    auto &document = view.Document();

    auto click = [&](boo::IntPoint at)
    {
      view.MouseMoved(at);
      view.MousePressed(at, pal::MouseButton::Left);
      view.MouseReleased(at, pal::MouseButton::Left);
      view.UpdateRendering();
    };
    auto ctrlShift = [&](pal::Key shift, Maybe<pal::Key> between = null)
    {
      (void)view.KeyPressed(pal::Key::Control, {pal::KeyModifier::Control});
      (void)view.KeyPressed(shift, {pal::KeyModifier::Control, pal::KeyModifier::Shift});
      if (between)
      {
        (void)view.KeyPressed(*between, {pal::KeyModifier::Control, pal::KeyModifier::Shift});
        (void)view.KeyReleased(*between, {pal::KeyModifier::Control, pal::KeyModifier::Shift});
      }
      (void)view.KeyReleased(shift, {pal::KeyModifier::Control});
      (void)view.KeyReleased(pal::Key::Control, {});
      view.UpdateRendering();
    };
    auto dirOf = [&](utf8_stringview id)
    {
      auto dir = document.GetElementById(boo::dom::DOMString(id))->GetAttribute(u8"dir");
      return dir ? std::string(dir->begin(), dir->end()) : std::string("none");
    };
    auto isRightToLeft = [](const boo::dom::Node &node)
    {
      return node.Renderer()->Style().GetWritingMode().BidiDirection() == boo::TextDirection::RTL;
    };

    SECTION("A text field's dir")
    {
      click({50, 10});
      ctrlShift(pal::Key::RightShift);
      CHECK(dirOf(u8"f") == "rtl");
      ctrlShift(pal::Key::Shift);
      CHECK(dirOf(u8"f") == "ltr");

      // Another key between cancels it.
      ctrlShift(pal::Key::RightShift, pal::Key::A);
      CHECK(dirOf(u8"f") == "ltr");
    }

    SECTION("A paragraph's direction")
    {
      // The editing host's content is wrapped in a block that takes the direction, so the text is looked for
      // each time.
      auto textBlockIsRightToLeft = [&]
      {
        boo::dom::Node *node = document.GetElementById(u8"p").get();
        while (node->FirstChild())
        {
          node = node->FirstChild();
        }
        return isRightToLeft(*node->Renderer()->ContainingBlock()->GetElement());
      };
      click({50, 30});
      CHECK_FALSE(textBlockIsRightToLeft());
      ctrlShift(pal::Key::RightShift);
      CHECK(textBlockIsRightToLeft());
      ctrlShift(pal::Key::Shift);
      CHECK_FALSE(textBlockIsRightToLeft());
    }

    SECTION("A text field doesn't take its direction from a page's keys it doesn't hold")
    {
      ctrlShift(pal::Key::RightShift);
      CHECK(dirOf(u8"f") == "none");
    }

    SECTION("Making selected text right to left embeds it")
    {
      auto &paragraph = *document.GetElementById(u8"p");
      auto &text = *paragraph.FirstChild();
      auto &selection = *document.GetSelection();
      REQUIRE_FALSE(selection.SetBaseAndExtent(text, 6, text, 11).HasException());
      CHECK(document.GetEditor().GetCommand(u8"MakeTextWritingDirectionRightToLeft").Execute());
      view.UpdateRendering();

      // "world" is in an element of its own now, embedded right to left.
      auto *last = paragraph.LastChild();
      REQUIRE(Is<boo::dom::Element>(last));
      CHECK(isRightToLeft(*last));
      CHECK(last->Renderer()->Style().GetUnicodeBidi() == boo::UnicodeBidi::Embed);
      CHECK(paragraph.TextContent() == boo::dom::DOMString(u8"hello world"));
      CHECK_FALSE(isRightToLeft(paragraph));
    }
  }

  TEST_CASE("Bidi editing: the editor page's right-to-left row", "[UI][Editing][Bidi]")
  {
    View view({1'200, 600});
    REQUIRE(LoadAndSettle(view, gfx::tests::GoldenDirectory() / ".." / ".." / "Krystal.Editor" / "pages"
                                  / "index.html"));
    auto &document = view.Document();
    auto byId = [&](utf8_stringview id)
    {
      auto element = document.GetElementById(boo::dom::DOMString(id));
      REQUIRE(element);
      return element;
    };
    auto isRightToLeft = [](const boo::dom::Element &element)
    {
      return element.Renderer()
             && element.Renderer()->Style().GetWritingMode().BidiDirection() == boo::TextDirection::RTL;
    };
    auto update = [&]
    {
      view.UpdateRendering();
      boo::html::EventLoop::Current().RunUntilIdle();
    };

    SECTION("the field turns right to left as Hebrew is typed, and the direction keys turn it")
    {
      auto field = byId(u8"bidi-field");
      Downcast<boo::html::HTMLElement>(*field).Focus();
      update();
      CHECK_FALSE(isRightToLeft(*field));
      CHECK(view.InsertText(u8"\u05e9\u05dc\u05d5\u05dd"));
      update();
      CHECK(isRightToLeft(*field));

      // Ctrl with the left Shift makes it left to right, whatever it holds.
      (void)view.KeyPressed(pal::Key::Control, {pal::KeyModifier::Control});
      (void)view.KeyPressed(pal::Key::Shift, {pal::KeyModifier::Control, pal::KeyModifier::Shift});
      (void)view.KeyReleased(pal::Key::Shift, {pal::KeyModifier::Control});
      (void)view.KeyReleased(pal::Key::Control, {});
      update();
      CHECK(field->GetAttribute(u8"dir") == boo::dom::DOMString(u8"ltr"));
      CHECK_FALSE(isRightToLeft(*field));
    }

    SECTION("the text area starts right to left, from its first letter")
    {
      CHECK(isRightToLeft(*byId(u8"bidi-area")));
    }

    SECTION("a Hebrew name is isolated, right to left, in its left-to-right item")
    {
      auto name = byId(u8"bidi-name");
      CHECK(isRightToLeft(*name));
      CHECK_FALSE(isRightToLeft(*name->ParentElement()));
    }

    SECTION("in the Hebrew paragraph, the left arrow moves the caret on")
    {
      auto &paragraph = *byId(u8"bidi-text")->LastElementChild();
      CHECK(isRightToLeft(paragraph));
      auto &text = *paragraph.FirstChild();
      auto &selection = *document.GetSelection();
      REQUIRE_FALSE(selection.Collapse(&text, 0).HasException());
      (void)view.KeyPressed(pal::Key::LeftArrow);
      update();
      CHECK(selection.FocusOffset() > 0u);
    }
  }
}
