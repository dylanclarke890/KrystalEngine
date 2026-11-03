#include "Krystal.Booey/DOM/Nodes/ProcessingInstruction.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/MutationObservers/MutationObserver.hpp"
#include "Krystal.Booey/DOM/MutationObservers/MutationRecord.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"

namespace krys::boo::dom::tests
{
  namespace
  {
    struct CommonTestData
    {
      Ref<Document> Document;
      Ref<ProcessingInstruction> Node;

      CommonTestData(dom::DOMString &&target, dom::DOMString &&value)
          : Document(CreateRef<dom::Document>()),
            Node(*Document->CreateProcessingInstruction(krys::move(target), krys::move(value)))
      {
      }
    };
  }

  TEST_CASE("Create a processing instruction node: initialized with the target and data, which must be valid",
            "[DOM][ProcessingInstruction][dom:interface-processinginstruction]"
            "[dom:create-a-processing-instruction-node][dom:processinginstruction-initialize]"
            "[dom:dom-processinginstruction-processinginstruction][dom:dom-processinginstruction-target]"
            "[dom:concept-pi-target]")
  {
    auto document = CreateRef<Document>();
    auto pi = ProcessingInstruction::Create(*document, u8"xml-stylesheet", u8"href=\"a.css\"").ReleaseValue();
    CHECK(pi->Target() == u8"xml-stylesheet");
    CHECK(pi->Data() == u8"href=\"a.css\"");
    CHECK(&pi->NodeDocument() == document.get());
    // Initialize updates the attributes from the data.
    CHECK(pi->GetAttribute(u8"href") == u8"a.css");

    for (auto [target, data] : {std::pair<DOMStringView, DOMStringView> {u8"1x", u8""},
                                {u8"", u8""},
                                {u8"a b", u8""},
                                {u8"x", u8"a ?> b"}})
    {
      auto result = ProcessingInstruction::Create(*document, DOMString(target), DOMString(data));
      REQUIRE(result.HasException());
      CHECK(result.GetException().Code() == ExceptionCode::InvalidCharacterError);
    }
    // ? and > apart are fine.
    CHECK_FALSE(ProcessingInstruction::Create(*document, u8"x", u8"a ? > b").HasException());
  }

  TEST_CASE("Update attributes from data: the parsed pseudo-attributes, or none if they don't parse",
            "[DOM][ProcessingInstruction][dom:update-attributes-from-data][dom:concept-pi-attribute-map]"
            "[dom:get-a-processing-instruction-attribute][dom:concept-cd-replace]")
  {
    CommonTestData data(u8"x", u8"b=\"2\" a='1 &amp; 2'");
    CHECK(data.Node->GetAttributeNames() == List<DOMString> {u8"b", u8"a"});
    CHECK(data.Node->GetAttribute(u8"a") == u8"1 & 2");
    CHECK_FALSE(data.Node->GetAttribute(u8"c").has_value());

    SECTION("Changing the data updates them")
    {
      REQUIRE_FALSE(data.Node->Data(u8"c=\"3\"").HasException());
      CHECK(data.Node->GetAttributeNames() == List<DOMString> {u8"c"});
    }

    SECTION("Data that doesn't parse clears them")
    {
      REQUIRE_FALSE(data.Node->AppendData(u8" junk").HasException());
      CHECK_FALSE(data.Node->HasAttributes());
      CHECK(data.Node->GetAttributeNames().empty());
    }
  }

  TEST_CASE("Update data from attributes: the attributes, double-quoted and escaped, replace the data",
            "[DOM][ProcessingInstruction][dom:update-data-from-attributes]"
            "[dom:dom-processinginstruction-setattribute]")
  {
    CommonTestData data(u8"x", u8"  a = '1'  ");

    auto observer = CreateRef<MutationObserver>([](const List<Ref<MutationRecord>> &, MutationObserver &) {});
    REQUIRE_FALSE(
      observer
        ->Observe(*data.Node, MutationObserverInit {.CharacterData = true, .CharacterDataOldValue = true})
        .HasException());

    REQUIRE_FALSE(data.Node->SetAttribute(u8"b", u8"<&>\"'").HasException());
    CHECK(data.Node->Data() == u8"a=\"1\" b=\"&lt;&amp;&gt;&quot;'\"");
    CHECK(data.Node->GetAttribute(u8"b") == u8"<&>\"'");

    auto records = observer->TakeRecords();
    REQUIRE(records.size() == 1uz);
    CHECK(records[0]->OldValue() == u8"  a = '1'  ");

    SECTION("Setting an existing attribute keeps its place")
    {
      REQUIRE_FALSE(data.Node->SetAttribute(u8"a", u8"2").HasException());
      CHECK(data.Node->Data() == u8"a=\"2\" b=\"&lt;&amp;&gt;&quot;'\"");
    }

    SECTION("The attributes aren't parsed again from the data")
    {
      // A valid attribute local name, but not an XML name, so the data no longer parses.
      REQUIRE_FALSE(data.Node->SetAttribute(u8"1x", u8"").HasException());
      CHECK(data.Node->Data() == u8"a=\"1\" b=\"&lt;&amp;&gt;&quot;'\" 1x=\"\"");
      CHECK(data.Node->GetAttributeNames() == List<DOMString> {u8"a", u8"b", u8"1x"});
    }

    SECTION("An invalid attribute local name throws an InvalidCharacterError")
    {
      for (auto name : {u8"", u8"a b", u8"a=b", u8"a/b", u8"a>b"})
      {
        auto result = data.Node->SetAttribute(DOMStringView(name), u8"");
        REQUIRE(result.HasException());
        CHECK(result.GetException().Code() == ExceptionCode::InvalidCharacterError);
      }
      CHECK(data.Node->GetAttributeNames() == List<DOMString> {u8"a", u8"b"});
    }
  }

  TEST_CASE("ProcessingInstruction.hasAttributes(), getAttributeNames(), getAttribute() and hasAttribute()",
            "[DOM][ProcessingInstruction][dom:dom-processinginstruction-hasattributes]"
            "[dom:dom-processinginstruction-getattributenames][dom:dom-processinginstruction-getattribute]"
            "[dom:dom-processinginstruction-hasattribute]")
  {
    CommonTestData data(u8"x", u8"");
    CHECK_FALSE(data.Node->HasAttributes());
    CHECK(data.Node->GetAttributeNames().empty());

    REQUIRE_FALSE(data.Node->Data(u8"href='a' Type=\"b\"").HasException());
    CHECK(data.Node->HasAttributes());
    CHECK(data.Node->GetAttributeNames() == List<DOMString> {u8"href", u8"Type"});
    CHECK(data.Node->GetAttribute(u8"Type") == u8"b");
    // Names match case-sensitively.
    CHECK_FALSE(data.Node->GetAttribute(u8"type").has_value());
    CHECK(data.Node->HasAttribute(u8"href"));
    CHECK_FALSE(data.Node->HasAttribute(u8"HREF"));
  }

  TEST_CASE("ProcessingInstruction.removeAttribute(): removes it, and updates the data",
            "[DOM][ProcessingInstruction][dom:dom-processinginstruction-removeattribute]")
  {
    CommonTestData data(u8"x", u8"a='1' b='2'");
    data.Node->RemoveAttribute(u8"a");
    CHECK(data.Node->Data() == u8"b=\"2\"");
    CHECK(data.Node->GetAttributeNames() == List<DOMString> {u8"b"});

    // Removing one it doesn't have still rewrites the data.
    REQUIRE_FALSE(data.Node->Data(u8"b = '2'").HasException());
    data.Node->RemoveAttribute(u8"missing");
    CHECK(data.Node->Data() == u8"b=\"2\"");
  }

  TEST_CASE("ProcessingInstruction.toggleAttribute(): adds or removes it, as force says",
            "[DOM][ProcessingInstruction][dom:dom-processinginstruction-toggleattribute]")
  {
    CommonTestData data(u8"x", u8"");
    auto toggle = [&](DOMStringView name, Maybe<bool> force)
    {
      auto result = data.Node->ToggleAttribute(name, force);
      REQUIRE_FALSE(result.HasException());
      return result.Value();
    };

    CHECK(toggle(u8"a", null));
    CHECK(data.Node->Data() == u8"a=\"\"");
    CHECK_FALSE(toggle(u8"a", null));
    CHECK(data.Node->Data() == u8"");

    CHECK(toggle(u8"a", true));
    CHECK(toggle(u8"a", true));
    CHECK(data.Node->Data() == u8"a=\"\"");
    CHECK_FALSE(toggle(u8"b", false));
    CHECK_FALSE(toggle(u8"a", false));
    CHECK_FALSE(data.Node->HasAttributes());

    auto result = data.Node->ToggleAttribute(u8"a b", null);
    REQUIRE(result.HasException());
    CHECK(result.GetException().Code() == ExceptionCode::InvalidCharacterError);
  }

  TEST_CASE("Clone a single node: a processing instruction's copy has its target, data and attributes",
            "[DOM][ProcessingInstruction][dom:clone-a-single-node]")
  {
    CommonTestData data(u8"x", u8"a='1'");
    auto clone = data.Node->CloneNode(false).ReleaseValue();
    auto &copy = Downcast<ProcessingInstruction>(*clone);
    CHECK(copy.Target() == u8"x");
    CHECK(copy.Data() == u8"a='1'");
    CHECK(copy.GetAttribute(u8"a") == u8"1");
  }

  TEST_CASE("Node.nodeName: a ProcessingInstruction's is its target",
            "[DOM][ProcessingInstruction][dom:dom-node-nodename]")
  {
    CommonTestData data(u8"test-target", u8"Hello, world!");
    REQUIRE(data.Node->NodeName() == u8"test-target");
  }

  TEST_CASE("Node.nodeType: a ProcessingInstruction is a PROCESSING_INSTRUCTION_NODE",
            "[DOM][ProcessingInstruction][dom:dom-node-nodetype][dom:dom-node-processing_instruction_node]")
  {
    CommonTestData data(u8"test-target", u8"Hello, world!");
    REQUIRE(data.Node->NodeType() == NodeType::PROCESSING_INSTRUCTION_NODE);
  }
}
