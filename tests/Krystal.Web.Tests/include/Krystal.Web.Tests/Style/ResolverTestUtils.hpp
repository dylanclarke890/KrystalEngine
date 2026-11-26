#pragma once

#include "Krystal.Core/Assert.hpp"
#include "Krystal.Web.Tests/Css/ValueTestUtils.hpp"
#include "Krystal.Web.Tests/Style/BuilderTestUtils.hpp"
#include "Krystal.Web/Css/Values/SerialisationContext.hpp"
#include "Krystal.Web/Dom/Nodes/HTMLDocument.hpp"
#include "Krystal.Web/Html/Elements/HTMLBodyElement.hpp"
#include "Krystal.Web/Html/EventLoop/EventLoop.hpp"
#include "Krystal.Web/Html/Parser/DocumentParser.hpp"
#include "Krystal.Web/Page/Frame.hpp"
#include "Krystal.Web/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Web/Style/ResolvedStyle.hpp"
#include "Krystal.Web/Style/Resolver.hpp"
#include "Krystal.Web/Style/Scope/Scope.hpp"
#include "Krystal.Web/Style/Values/Primitives/KeywordSerialisation.hpp"
#include "Krystal.Web/Style/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Web/Style/Values/Types.hpp"
#include <string>

namespace krys::web::style::tests
{
  /// @brief Whether a fixture's document is shown in a frame with a view, so it has a render tree.
  enum class WithView : bool
  {
    No,
    Yes,
  };

  /// @brief Fixture for tests that resolve the styles of a parsed document's elements. The document is shown
  /// in an 800x600 view unless the test asks for none.
  struct ResolverFixture : css::tests::ValueFixture
  {
    Ref<HTMLDocument> Document;
    RefPtr<web::Frame> Frame;

    explicit ResolverFixture(utf8_string &&markup, WithView withView = WithView::Yes) noexcept
        : Document(Parse(krys::move(markup)))
    {
      Document->StyleScope().FlushPendingUpdate();
      if (withView == WithView::Yes)
      {
        Frame = web::Frame::Create();
        Frame->CreateView({800, 600});
        Frame->SetDocument(ShareRefPtr<web::Document>(Document.get()));
      }
    }

    KRYS_NODISCARD static Ref<HTMLDocument> Parse(utf8_string &&markup) noexcept
    {
      html::EventLoop::Current().RunUntilIdle();
      auto document = CreateRef<HTMLDocument>();
      auto parser = CreateRef<html::DocumentParser>(*document);
      parser->InputStream().Append(krys::move(markup), IsEOF::Yes);
      parser->PumpTokenizer();
      return document;
    }

    KRYS_NODISCARD Resolver &GetResolver() noexcept
    {
      return Document->StyleScope().GetResolver();
    }

    KRYS_NODISCARD Element &ById(DOMStringView id) noexcept
    {
      auto element = Document->GetElementById(id);
      krys_assert(element);
      return *element;
    }

    /// @brief The resolved style of @p element, resolving its ancestors' styles first as the tree resolver
    /// does: each element inherits from its parent's style, and root-relative units use the document
    /// element's.
    KRYS_NODISCARD ResolvedStyle
      Resolve(Element &element, RuleMatchingBehavior behavior = RuleMatchingBehavior::MatchAllRules) noexcept
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
