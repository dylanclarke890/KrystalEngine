#include "Krystal.Booey/DOM/Iterators/ComposedTreeIterator.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/DOM/Iterators/ComposedTreeAncestorIterator.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"

namespace krys::boo::dom::tests
{
  using style::tests::ResolverFixture;

  namespace
  {
    KRYS_NODISCARD Ref<Element> Create(Document &document, DOMStringAtom localName)
    {
      auto element = document.CreateElement(localName);
      REQUIRE_FALSE(element.HasException());
      return element.ReleaseValue();
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "ComposedTreeIterator", "[DOM][ComposedTree]")
  {
    auto document = ResolverFixture::Parse(
      u8R"(<!DOCTYPE html><body><div id=host><span id=a></span><b id=unassigned slot=x></b>text</div><p id=after></p>)");
    auto &host = *document->GetElementById(u8"host");
    auto shadow = host.AttachShadow({}).ReleaseValue();
    auto inner = Create(*document, u8"i");
    auto slot = Create(*document, u8"slot");
    REQUIRE_FALSE(shadow->AppendChild(*inner).HasException());
    REQUIRE_FALSE(shadow->AppendChild(*slot).HasException());

    auto &body = *document->Body();
    // The shadow tree replaces the host's children; the default slot takes the unnamed slottables (the text
    // too), and the element assigned to no slot isn't in the composed tree.
    CHECK(ComposedTreeAsText(body)
          == u8"  div (shadow root)\n    i\n    slot\n      span\n      #text\n  p\n");

    SECTION("Depth and traversal")
    {
      auto descendants = ComposedTreeDescendants(body);
      auto it = descendants.begin();
      CHECK(&*it == &host);
      CHECK(it.Depth() == 1);
      it.TraverseNextSkippingChildren();
      CHECK(&*it == document->GetElementById(u8"after").get());
      it.TraverseNext();
      CHECK(it == descendants.end());

      auto at = descendants.at(*document->GetElementById(u8"a"));
      CHECK(&*at == document->GetElementById(u8"a").get());
      CHECK(at.Depth() == 3);
    }

    SECTION("Children")
    {
      List<Node *> children;
      for (auto &child : ComposedTreeChildren(host))
      {
        children.push_back(&child);
      }
      CHECK(children == List<Node *> {inner.get(), slot.get()});
    }

    SECTION("Ancestors")
    {
      auto &a = *document->GetElementById(u8"a");
      List<Element *> ancestors;
      for (auto &ancestor : ComposedTreeAncestors(a))
      {
        ancestors.push_back(&ancestor);
      }
      CHECK(ancestors == List<Element *> {slot.get(), &host, &body, document->DocumentElement().get()});
      CHECK(ComposedTreeAncestors(*shadow).First() == &host);
      CHECK(a.ParentInComposedTree() == slot.get());
      CHECK(a.IsComposedTreeDescendantOf(host));
      // An unassigned child's composed tree parent is its host, though it isn't in the composed tree.
      CHECK(document->GetElementById(u8"unassigned")->ParentElementInComposedTree() == &host);
      CHECK_FALSE(ComposedTreeAncestors(*document->GetElementById(u8"unassigned")).First());
    }
  }
}
