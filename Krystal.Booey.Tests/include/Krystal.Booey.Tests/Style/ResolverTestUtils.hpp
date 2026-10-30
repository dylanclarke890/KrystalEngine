#pragma once

#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey.Tests/Style/BuilderTestUtils.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLDocumentParser.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/ResolvedStyle.hpp"
#include "Krystal.Booey/Style/Resolver.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordSerialisation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"
#include "Krystal.Core/Assert.hpp"
#include <string>

namespace krys::boo::style::tests
{
  /// @brief Whether a fixture's document is shown in a frame with a view, so it has a render tree.
  enum class WithView : bool
  {
    No,
    Yes
  };

  /// @brief Fixture for tests that resolve the styles of a parsed document's elements. The document is shown
  /// in an 800x600 view unless the test asks for none.
  struct ResolverFixture : css::tests::ValueFixture
  {
    Ref<dom::HTMLDocument> Document;
    RefPtr<LocalFrame> Frame;

    explicit ResolverFixture(utf8_string &&markup, WithView withView = WithView::Yes) noexcept
        : Document(Parse(krys::move(markup)))
    {
      Document->StyleScope().FlushPendingUpdate();
      if (withView == WithView::Yes)
      {
        Frame = LocalFrame::Create();
        Frame->CreateView({800, 600});
        Frame->SetDocument(ShareRefPtr<dom::Document>(Document.get()));
      }
    }

    KRYS_NODISCARD static Ref<dom::HTMLDocument> Parse(utf8_string &&markup) noexcept
    {
      html::EventLoop::Current().RunUntilIdle();
      auto document = CreateRef<dom::HTMLDocument>();
      auto parser = CreateRef<html::HTMLDocumentParser>(*document);
      parser->InputStream().Append(krys::move(markup), IsEOF(true));
      (void)parser->PumpTokenizer();
      return document;
    }

    KRYS_NODISCARD Resolver &GetResolver() noexcept
    {
      return Document->StyleScope().GetResolver();
    }

    KRYS_NODISCARD dom::Element &ById(dom::DOMStringView id) noexcept
    {
      auto element = Document->GetElementById(id);
      krys_assert(element);
      return *element;
    }

    /// @brief The resolved style of @p element, resolving its ancestors' styles first as the tree resolver
    /// does: each element inherits from its parent's style, and root-relative units use the document
    /// element's.
    KRYS_NODISCARD ResolvedStyle Resolve(
      dom::Element &element, RuleMatchingBehavior behavior = RuleMatchingBehavior::MatchAllRules) noexcept
    {
      auto documentElement = Document->DocumentElement();
      krys_assert(documentElement);
      if (&element == documentElement.get())
      {
        return GetResolver().StyleForElement(element, {Document->InitialContainingBlockStyle()}, behavior);
      }

      auto rootStyle = Resolve(*documentElement).Style;
      auto parent = element.ParentElement();
      krys_assert(parent);
      auto parentStyle =
        parent == documentElement.get() ? RenderStyle::ClonePtr(*rootStyle) : Resolve(*parent).Style;
      return GetResolver().StyleForElement(
        element, {.ParentStyle = parentStyle.get(), .DocumentElementStyle = rootStyle.get()}, behavior);
    }

    /// @brief Serialises a computed value of @p style.
    template <typename T>
    KRYS_NODISCARD static std::string Serialised(const RenderStyle &style, const T &value) noexcept
    {
      return css::tests::ToNarrow(Serialise(css::DefaultSerialisationContext(), style, value));
    }
  };
}
