#include "Krystal.UI/Config/Config.hpp"
#include "Krystal.UI/LayoutEngine.hpp"
#include "Krystal.UI/Node/Node.hpp"
#include <catch_all.hpp>
#include <functional>
#include <memory>
#include <vector>
namespace Krys::Tests
{
  using namespace Krys::UI;

  struct YGPersistentNodeCloningTest
  {
    struct NodeWrapper
    {
      explicit NodeWrapper(ConfigRef config, std::vector<std::shared_ptr<NodeWrapper>> children = {})
          : node {NodeCreate(config)}, children {std::move(children)}
      {
        NodeSetContext(node, this);

        auto privateNode = node;
        for (const auto &child : this->children)
        {
          auto privateChild = child->node;
          // Claim first ownership of not yet owned nodes, to avoid immediately
          // cloning them
          if (NodeGetOwner(child->node) == nullptr)
          {
            privateChild->SetOwner(privateNode);
          }
          privateNode->InsertChild(privateChild, privateNode->GetChildCount());
        }
      }

      // Clone, with current children, for mutation
      NodeWrapper(const NodeWrapper &other) : node {NodeClone(other.node)}, children {other.children}
      {
        NodeSetContext(node, this);

        auto privateNode = node;
        privateNode->SetOwner(nullptr);
      }

      // Clone, with new children
      NodeWrapper(const NodeWrapper &other, std::vector<std::shared_ptr<NodeWrapper>> children)
          : node {NodeClone(other.node)}, children {std::move(children)}
      {
        NodeSetContext(node, this);

        auto privateNode = node;
        privateNode->SetOwner(nullptr);
        privateNode->SetChildren({});
        privateNode->SetDirty(true);

        for (const auto &child : this->children)
        {
          auto privateChild = child->node;
          // Claim first ownership of not yet owned nodes, to avoid immediately
          // cloning them
          if (NodeGetOwner(child->node) == nullptr)
          {
            privateChild->SetOwner(privateNode);
          }
          privateNode->InsertChild(privateChild, privateNode->GetChildCount());
        }
      }

      NodeWrapper(NodeWrapper &&) = delete;

      ~NodeWrapper()
      {
        NodeDestroy(node);
      }

      NodeWrapper &operator=(const NodeWrapper &other) = delete;
      NodeWrapper &operator=(NodeWrapper &&other) = delete;

      NodeRef node;
      std::vector<std::shared_ptr<NodeWrapper>> children;
    };

    struct ConfigWrapper
    {
      ConfigWrapper()
      {
        ConfigSetCloneNodeFunc(config,
                               [](NodeConstRef oldNode, NodeConstRef owner, size_t childIndex)
                               {
                                 onClone(oldNode, owner, childIndex);
                                 auto wrapper = static_cast<NodeWrapper *>(NodeGetContext(owner));
                                 auto old = static_cast<NodeWrapper *>(NodeGetContext(oldNode));

                                 wrapper->children[childIndex] = std::make_shared<NodeWrapper>(*old);
                                 return wrapper->children[childIndex]->node;
                               });
        onClone = [](...) {};
      }

      ConfigWrapper(const ConfigWrapper &) = delete;
      ConfigWrapper(ConfigWrapper &&) = delete;

      ~ConfigWrapper()
      {
        ConfigDestroy(config);
      }

      ConfigWrapper &operator=(const ConfigWrapper &) = delete;
      ConfigWrapper &operator=(ConfigWrapper &&) = delete;

      ConfigRef config {ConfigCreate()};
    };

    ConfigWrapper configWrapper;
    ConfigRef config {configWrapper.config};

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
    static inline std::function<void(NodeConstRef, NodeConstRef, size_t)> onClone;
  };

  TEST_CASE_METHOD(YGPersistentNodeCloningTest, "changing_sibling_height_does_not_clone_neighbors",
                   "[LayoutEngine]")
  {
    // <ScrollView>
    //   <View id="Sibling" style={{ height: 1 }} />
    //   <View id="A" style={{ height: 1 }}>
    //     <View id="B">
    //       <View id="C">
    //         <View id="D"/>
    //       </View>
    //     </View>
    //   </View>
    // </ScrollView>

    auto sibling = std::make_shared<NodeWrapper>(config);
    NodeStyleSetHeight(sibling->node, 1);

    auto d = std::make_shared<NodeWrapper>(config);
    auto c = std::make_shared<NodeWrapper>(config, std::vector {d});
    auto b = std::make_shared<NodeWrapper>(config, std::vector {c});
    auto a = std::make_shared<NodeWrapper>(config, std::vector {b});
    NodeStyleSetHeight(a->node, 1);

    auto scrollContentView = std::make_shared<NodeWrapper>(config, std::vector {sibling, a});
    NodeStyleSetPositionType(scrollContentView->node, PositionType::Absolute);

    auto scrollView = std::make_shared<NodeWrapper>(config, std::vector {scrollContentView});
    NodeStyleSetWidth(scrollView->node, 100);
    NodeStyleSetHeight(scrollView->node, 100);

    // We don't expect any cloning during the first layout
    onClone = [](...)
    {
      FAIL();
    };

    NodeCalculateLayout(scrollView->node, Undefined, Undefined, Direction::LTR);

    auto siblingPrime = std::make_shared<NodeWrapper>(config);
    NodeStyleSetHeight(siblingPrime->node, 2);

    auto scrollContentViewPrime =
      std::make_shared<NodeWrapper>(*scrollContentView, std::vector {siblingPrime, a});
    auto scrollViewPrime = std::make_shared<NodeWrapper>(*scrollView, std::vector {scrollContentViewPrime});

    std::vector<NodeWrapper *> nodesCloned;
    // We should only need to clone "A"
    onClone = [&](NodeConstRef oldNode, NodeConstRef /*owner*/, size_t /*childIndex*/)
    {
      nodesCloned.push_back(static_cast<NodeWrapper *>(NodeGetContext(oldNode)));
    };

    NodeCalculateLayout(scrollViewPrime->node, Undefined, Undefined, Direction::LTR);

    REQUIRE(nodesCloned.size() == 1);
    REQUIRE(nodesCloned[0] == a.get());
  }

  TEST_CASE_METHOD(YGPersistentNodeCloningTest, "clone_leaf_display_contents_node", "[LayoutEngine]")
  {
    // <View id="A">
    //   <View id="B" style={{ display: 'contents' }} />
    // </View>

    auto b = std::make_shared<NodeWrapper>(config);
    auto a = std::make_shared<NodeWrapper>(config, std::vector {b});
    NodeStyleSetDisplay(b->node, DisplayType::Contents);

    // We don't expect any cloning during the first layout
    onClone = [](...)
    {
      FAIL();
    };

    NodeCalculateLayout(a->node, Undefined, Undefined, Direction::LTR);

    auto aPrime = std::make_shared<NodeWrapper>(config, std::vector {b});

    std::vector<NodeWrapper *> nodesCloned;
    // We should clone "C"
    onClone = [&](NodeConstRef oldNode, NodeConstRef /*owner*/, size_t /*childIndex*/)
    {
      nodesCloned.push_back(static_cast<NodeWrapper *>(NodeGetContext(oldNode)));
    };

    NodeCalculateLayout(aPrime->node, 100, 100, Direction::LTR);

    REQUIRE(nodesCloned.size() == 1);
    REQUIRE(nodesCloned[0] == b.get());
  }
}