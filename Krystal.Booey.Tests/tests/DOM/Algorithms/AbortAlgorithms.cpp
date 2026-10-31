#include "Krystal.Booey/DOM/Algorithms/AbortAlgorithms.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/AbortController.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/DOM/EventListenerHandle.hpp"
#include "Krystal.Booey/DOM/Types/Exception.hpp"

namespace krys::boo::dom::tests
{
  TEST_CASE("Add: appends an algorithm once, unless the signal is aborted",
            "[AbortAlgorithms][dom:abortsignal-add]")
  {
    AbortController controller;
    int runs = 0;
    AbortAlgorithmCallback algorithm = [&](const Any &)
    {
      ++runs;
    };

    SECTION("An algorithm added twice runs once")
    {
      AbortAlgorithms::Add(algorithm, *controller.Signal());
      AbortAlgorithms::Add(algorithm, *controller.Signal());
      controller.Abort();
      CHECK(runs == 1);
    }

    SECTION("An aborted signal doesn't take it")
    {
      auto signal = AbortSignal::Abort();
      AbortAlgorithms::Add(algorithm, *signal);
      AbortAlgorithms::SignalAbort(*signal);
      CHECK(runs == 0);
    }
  }

  TEST_CASE("Remove: the algorithm no longer runs", "[AbortAlgorithms][dom:abortsignal-remove]")
  {
    AbortController controller;
    int runs = 0;
    AbortAlgorithmCallback algorithm = [&](const Any &)
    {
      ++runs;
    };
    AbortAlgorithms::Add(algorithm, *controller.Signal());
    AbortAlgorithms::Remove(algorithm, *controller.Signal());
    controller.Abort();
    CHECK(runs == 0);
  }

  TEST_CASE("Signal abort: sets the reason once, then runs the abort steps for the signal and its dependents",
            "[AbortAlgorithms][dom:abortsignal-signal-abort]")
  {
    AbortController controller;
    auto &signal = *controller.Signal();

    SECTION("An empty reason (undefined) counts as not given")
    {
      AbortAlgorithms::SignalAbort(signal, Any());
      REQUIRE(signal.Aborted());
      auto *reason = std::any_cast<Exception>(&signal.Reason());
      REQUIRE(reason);
      CHECK(reason->Code() == ExceptionCode::AbortError);
    }

    SECTION("An aborted signal keeps its reason")
    {
      AbortAlgorithms::SignalAbort(signal, Any(1));
      AbortAlgorithms::SignalAbort(signal, Any(2));
      CHECK(std::any_cast<int>(signal.Reason()) == 1);
    }

    SECTION("Dependents are aborted before the signal's abort steps and run theirs after")
    {
      auto dependent = AbortSignal::Any({ShareRef(signal)});
      List<std::string> log;
      AbortAlgorithms::Add(
        [&](const Any &)
        {
          log.push_back("signal algorithm");
          CHECK(dependent->Aborted());
          CHECK(std::any_cast<int>(dependent->Reason()) == 7);
        },
        signal);
      AbortAlgorithms::Add([&](const Any &) { log.push_back("dependent algorithm"); }, *dependent);
      auto signalListener =
        signal.AddEventListener(EventNames::Abort, [&](Event &) { log.push_back("signal event"); });
      auto dependentListener =
        dependent->AddEventListener(EventNames::Abort, [&](Event &) { log.push_back("dependent event"); });

      AbortAlgorithms::SignalAbort(signal, Any(7));
      CHECK(
        log
        == List<std::string> {"signal algorithm", "signal event", "dependent algorithm", "dependent event"});
    }
  }

  TEST_CASE("Run the abort steps: runs and empties the abort algorithms, then fires abort",
            "[AbortAlgorithms][dom:run-the-abort-steps][dom:abortsignal-abort-algorithms]")
  {
    AbortController controller;
    auto &signal = *controller.Signal();
    List<std::string> log;

    SECTION("Algorithms run in order, with the reason, before the event")
    {
      AbortAlgorithms::Add(
        [&](const Any &reason)
        {
          log.push_back("first");
          CHECK(std::any_cast<int>(reason) == 4);
        },
        signal);
      AbortAlgorithms::Add([&](const Any &) { log.push_back("second"); }, signal);
      auto listener = signal.AddEventListener(EventNames::Abort, [&](Event &) { log.push_back("event"); });

      controller.Abort(Any(4));
      CHECK(log == List<std::string> {"first", "second", "event"});
    }

    SECTION("An algorithm that removes itself doesn't skip the next")
    {
      Maybe<AbortAlgorithmCallback> self;
      self = AbortAlgorithmCallback(
        [&](const Any &)
        {
          log.push_back("first");
          AbortAlgorithms::Remove(*self, signal);
        });
      AbortAlgorithms::Add(*self, signal);
      AbortAlgorithms::Add([&](const Any &) { log.push_back("second"); }, signal);

      controller.Abort();
      CHECK(log == List<std::string> {"first", "second"});
    }
  }
}
