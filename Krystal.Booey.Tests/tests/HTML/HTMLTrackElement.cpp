#include "Krystal.Booey/HTML/HTMLTrackElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"

namespace krys::boo::html::tests
{
  TEST_CASE(
    "HTMLTrackElement.kind: reflects its known values, subtitles when missing and metadata when invalid",
    "[HTML][HTMLTrackElement][html:dom-track-kind][html:attr-track-kind]")
  {
    auto document = CreateRef<dom::HTMLDocument>();
    auto element = document->CreateElement(u8"track").ReleaseValue();
    auto &track = Downcast<HTMLTrackElement>(Downcast<HTMLElement>(*element));

    CHECK(css::tests::ToNarrow(track.Kind()) == "subtitles");
    track.Kind(u8"CAPTIONS");
    CHECK(css::tests::ToNarrow(track.Kind()) == "captions");
    track.Kind(u8"karaoke");
    CHECK(css::tests::ToNarrow(track.Kind()) == "metadata");
    CHECK(element->GetAttribute(u8"kind") == u8"karaoke");

    // A track that isn't loaded has no readiness.
    CHECK(track.ReadyState() == TrackReadyState::None);
  }
}
