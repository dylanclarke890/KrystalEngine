#include "Krystal.Booey/HTML/Elements/HTMLMarqueeElement.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "catch2/generators/catch_generators.hpp"
#include "Krystal.Booey.Tests/Animation/AnimationTestUtils.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/Animation/KeyframeEffect.hpp"
#include "Krystal.Booey/Animation/WebAnimation.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLDivElement.hpp"
#include <cmath>
#include <set>
#include <string>

namespace krys::boo::tests
{
  namespace
  {
    using namespace std::chrono_literals;

    /// @brief A 400x40 marquee with @p attributes around a 100x20 box, in a page whose animations run.
    struct MarqueeFixture : AnimationPageFixture
    {
      explicit MarqueeFixture(utf8_stringview attributes,
                              utf8_stringview size = u8"width: 400px; height: 40px")
          : AnimationPageFixture(utf8_string(u8"<!DOCTYPE html><body style='margin: 0'><marquee id=m ")
                                 + utf8_string(attributes) + u8" style='" + utf8_string(size)
                                 + u8"'><span style='display: inline-block; width: 100px; height: 20px; "
                                   u8"vertical-align: top'></span>"
                                   u8"</marquee>")
      {
        StartAnimations();
      }

      KRYS_NODISCARD html::HTMLMarqueeElement &Marquee() noexcept
      {
        return Downcast<html::HTMLMarqueeElement>(ById(u8"m"));
      }

      /// @brief The mover's animations, oldest first.
      KRYS_NODISCARD List<RefPtr<WebAnimation>> Animations() noexcept
      {
        auto *mover = Marquee().Mover();
        REQUIRE(mover);
        return mover->GetAnimations();
      }

      /// @brief The latest animation's keyframes' transforms, and its duration.
      KRYS_NODISCARD std::string Loop() noexcept
      {
        auto animations = Animations();
        if (animations.empty())
        {
          return "none";
        }
        auto *effect = animations.back()->GetKeyframeEffect();
        REQUIRE(effect);
        std::string text;
        for (auto &keyframe : effect->GetKeyframes())
        {
          auto &transform = keyframe.StyleStrings[css::PropertyId::Transform];
          text += std::string(transform.begin(), transform.end()) + " ";
        }
        auto duration = std::get<double>(effect->GetBindingsTiming().Duration);
        return text + std::to_string(static_cast<int64>(std::round(duration)));
      }
    };
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "HTMLMarqueeElement: its reflected attributes",
                   "[HTML][HTMLMarqueeElement][html:the-marquee-element]")
  {
    AnimationPageFixture fixture(u8"<!DOCTYPE html><marquee id=m></marquee>");
    auto &marquee = Downcast<html::HTMLMarqueeElement>(fixture.ById(u8"m"));
    CHECK(marquee.Loop() == -1);
    CHECK(marquee.ScrollAmount() == 6);
    CHECK(marquee.ScrollDelay() == 85);
    CHECK_FALSE(marquee.TrueSpeed());
    CHECK(marquee.HSpace() == 0);
    CHECK(marquee.VSpace() == 0);

    (void)marquee.SetAttribute(u8"scrollamount", u8"-3");
    (void)marquee.SetAttribute(u8"scrolldelay", u8"x");
    (void)marquee.SetAttribute(u8"loop", u8"0");
    CHECK(marquee.ScrollAmount() == 6);
    CHECK(marquee.ScrollDelay() == 85);
    CHECK(marquee.Loop() == -1);

    marquee.ScrollAmount(7);
    CHECK(marquee.GetAttribute(u8"scrollamount") == u8"7");
    marquee.HSpace(10);
    CHECK(marquee.GetAttribute(u8"hspace") == u8"10");
    marquee.TrueSpeed(true);
    CHECK(marquee.HasAttribute(u8"truespeed"));
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "HTMLMarqueeElement: loop takes a positive count or -1",
                   "[HTML][HTMLMarqueeElement][html:dom-marquee-loop][html:marquee-loop-count]")
  {
    AnimationPageFixture fixture(u8"<!DOCTYPE html><marquee id=m></marquee>");
    auto &marquee = Downcast<html::HTMLMarqueeElement>(fixture.ById(u8"m"));
    marquee.Loop(5);
    CHECK(marquee.Loop() == 5);
    CHECK(marquee.GetAttribute(u8"loop") == u8"5");
    // Other values are ignored.
    marquee.Loop(0);
    marquee.Loop(-2);
    CHECK(marquee.GetAttribute(u8"loop") == u8"5");
    marquee.Loop(-1);
    CHECK(marquee.Loop() == -1);
    CHECK(marquee.GetAttribute(u8"loop") == u8"-1");
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "HTMLMarqueeElement: the scroll interval",
                   "[HTML][HTMLMarqueeElement][html:marquee-scroll-interval]")
  {
    AnimationPageFixture fixture(u8"<!DOCTYPE html><marquee id=m></marquee>");
    auto &marquee = Downcast<html::HTMLMarqueeElement>(fixture.ById(u8"m"));
    CHECK(marquee.ScrollInterval() == 85);
    marquee.ScrollDelay(10);
    CHECK(marquee.ScrollInterval() == 60);
    marquee.TrueSpeed(true);
    CHECK(marquee.ScrollInterval() == 10);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture,
                   "HTMLMarqueeElement: each behavior and direction moves the mover",
                   "[HTML][HTMLMarqueeElement][html:the-marquee-element-2][html:marquee-scroll-distance]")
  {
    // As Chrome moves its contents: the distance at the scroll distance each scroll interval.
    struct Case
    {
      const char8_t *Attributes;
      const char8_t *Size;
      std::string Loop;
    };
    const char8_t *box = u8"width: 400px; height: 40px";
    auto test = GENERATE_COPY(values<Case>({
      {u8"", box, "translateX(400px) translateX(-100px) 7083"},
      {u8"scrollamount=20", box, "translateX(400px) translateX(-100px) 2125"},
      {u8"scrolldelay=10 truespeed", box, "translateX(400px) translateX(-100px) 833"},
      {u8"direction=right", box, "translateX(-100px) translateX(400px) 7083"},
      {u8"direction=up", box, "translateY(40px) translateY(-20px) 850"},
      {u8"behavior=alternate", box, "translateX(300px) translateX(0px) 4250"},
      {u8"behavior=alternate direction=down", box, "translateY(0px) translateY(20px) 283"},
      {u8"behavior=alternate", u8"width: 50px; height: 40px", "translateX(0px) translateX(-50px) 708"},
      {u8"behavior=slide", box, "translateX(400px) translateX(0px) 5667"},
      {u8"behavior=slide", u8"width: 50px; height: 40px", "translateX(50px) translateX(0px) 708"},
      {u8"scrollamount=0", box, "none"},
    }));
    INFO(reinterpret_cast<const char *>(test.Attributes));
    MarqueeFixture fixture(test.Attributes, test.Size);
    CHECK(fixture.Loop() == test.Loop);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "HTMLMarqueeElement: loops, alternating, until the loop count",
                   "[HTML][HTMLMarqueeElement][html:increment-the-marquee-current-loop-index]")
  {
    SECTION("an alternation goes back on odd loops")
    {
      MarqueeFixture fixture(u8"behavior=alternate scrollamount=300 scrolldelay=60");
      CHECK(fixture.Loop() == "translateX(300px) translateX(0px) 60");
      fixture.Frame(100ms);
      fixture.Frame();
      CHECK(fixture.Loop() == "translateX(0px) translateX(300px) 60");
      fixture.Frame(100ms);
      fixture.Frame();
      CHECK(fixture.Loop() == "translateX(300px) translateX(0px) 60");
    }

    SECTION("a loop count stops it")
    {
      MarqueeFixture fixture(u8"loop=2 scrollamount=500 scrolldelay=60");
      // Two loops, each its own animation, and no more.
      std::set<WebAnimation *> loops;
      for (int32 i = 0; i < 8; ++i)
      {
        for (auto &animation : fixture.Animations())
        {
          loops.insert(animation.get());
        }
        fixture.Frame(100ms);
      }
      CHECK(loops.size() == 2);
      auto animations = fixture.Animations();
      REQUIRE_FALSE(animations.empty());
      CHECK(animations.back()->GetPlayState() == WebAnimation::PlayState::Finished);
    }

    SECTION("a slide slides once")
    {
      MarqueeFixture fixture(u8"behavior=slide scrollamount=400 scrolldelay=60");
      std::set<WebAnimation *> loops;
      for (int32 i = 0; i < 8; ++i)
      {
        for (auto &animation : fixture.Animations())
        {
          loops.insert(animation.get());
        }
        fixture.Frame(100ms);
      }
      CHECK(loops.size() == 1);
      auto animations = fixture.Animations();
      REQUIRE(animations.size() == 1);
      CHECK(animations[0]->GetPlayState() == WebAnimation::PlayState::Finished);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "HTMLMarqueeElement: its contents move with the mover",
                   "[HTML][HTMLMarqueeElement][html:the-marquee-element-2]")
  {
    MarqueeFixture fixture(u8"scrollamount=50 scrolldelay=100");
    // 500px in a second: in from the right edge, and across at 0.5px a millisecond.
    auto &box = *fixture.Marquee().FirstElementChild();
    CHECK(box.GetBoundingClientRect()->X() == Catch::Approx(400).margin(1));
    fixture.Frame(500ms);
    CHECK(box.GetBoundingClientRect()->X() == Catch::Approx(150).margin(10));
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "HTMLMarqueeElement: stop() holds it, and start() plays it on",
                   "[HTML][HTMLMarqueeElement][html:dom-marquee-start][html:dom-marquee-stop]")
  {
    MarqueeFixture fixture(u8"");
    auto animations = fixture.Animations();
    REQUIRE(animations.size() == 1);
    auto animation = animations[0];
    CHECK(animation->GetPlayState() == WebAnimation::PlayState::Running);

    fixture.Marquee().Stop();
    CHECK(animation->GetPlayState() == WebAnimation::PlayState::Paused);
    fixture.Frame(1s);
    fixture.Marquee().Start();
    fixture.Frame();
    CHECK(animation->GetPlayState() == WebAnimation::PlayState::Running);
    CHECK(fixture.Animations().size() == 1);

    // Removed from its document, it stops; it isn't connected to move.
    fixture.Marquee().Remove();
    CHECK(animation->GetPlayState() == WebAnimation::PlayState::Paused);
  }
}
