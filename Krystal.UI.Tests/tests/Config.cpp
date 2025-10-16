#include "Krystal.UI/Config/Config.hpp"
#include "Krystal.UI/LayoutEngine.hpp"
#include "Krystal.UI/Node/Node.hpp"
#include <catch_all.hpp>
#include <functional>
#include <memory>

namespace Krys::Tests
{
  using namespace Krys::UI;

  struct ConfigCloningTest
  {
    std::unique_ptr<Config, std::function<void(ConfigRef)>> config;

    ConfigCloningTest()
    {
      config = {ConfigCreate(), ConfigDestroy};
    }

    ~ConfigCloningTest()
    {
      config.reset();
    }

    static Node clonedNode;

    static NodeRef CloneNode(NodeConstRef, NodeConstRef, size_t)
    {
      return &clonedNode;
    }

    static NodeRef DoNotClone(NodeConstRef, NodeConstRef, size_t)
    {
      return nullptr;
    }
  };
  Node ConfigCloningTest::clonedNode = {};

  TEST_CASE_METHOD(ConfigCloningTest, "uses values provided by cloning callback", "[ConfigCloning]")
  {
    config->SetCloneNodeCallback(CloneNode);

    Node node {};
    Node owner {};
    auto clone = config->CloneNode(&node, &owner, 0);
    REQUIRE(clone == &clonedNode);
  }

  TEST_CASE_METHOD(ConfigCloningTest, "falls_back_to_regular_cloning_if_callback_returns_null",
                   "[ConfigCloning]")
  {
    config->SetCloneNodeCallback(DoNotClone);

    Node node {};
    Node owner {};
    auto clone = config->CloneNode(&node, &owner, 0);

    REQUIRE(clone != nullptr);
    NodeDestroy(clone);
  }
}
