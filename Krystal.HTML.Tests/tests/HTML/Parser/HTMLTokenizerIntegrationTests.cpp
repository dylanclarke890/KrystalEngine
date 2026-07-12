#include "Krystal.HTML.Tests/ParserTestUtils.hpp"
#include "Krystal.HTML/HTML/Parser/HTMLTokenizer.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::HTML;

  INTEGRATION_TEST(
    "_ExampleMarkup1",
    (IntegrationTest {
      .Input = UR"html(<!DOCTYPE html>
        <html lang="en">
        <head>
            <meta charset="UTF-8">
            <meta name="viewport" content="width=device-width, initial-scale=1.0">
            <link type="text/css" href="index.css">
            <title>Document</title>
        </head>
        <body style="background-color: maroon;">
            <div style="
              box-sizing: border-box;">Hello, Krystal UI</div>
            <script type="javascript" src="index.js"></script>
        </body
>
        </html>)html",
      .Output =
        {
          CreateDOCTYPEToken({.Name = u8"html"}),
          CreateCharacterToken(u8"\n        "),
          CreateStartTagToken({.Name = u8"html", .Attributes = {{.Name = u8"lang", .Value = u8"en"}}}),
          CreateCharacterToken(u8"\n        "),
          CreateStartTagToken({.Name = u8"head"}),
          CreateCharacterToken(u8"\n            "),
          CreateStartTagToken({.Name = u8"meta", .Attributes = {{.Name = u8"charset", .Value = u8"UTF-8"}}}),
          CreateCharacterToken(u8"\n            "),
          CreateStartTagToken({.Name = u8"meta",
                               .Attributes = {{.Name = u8"name", .Value = u8"viewport"},
                                              {.Name = u8"content",
                                               .Value = u8"width=device-width, initial-scale=1.0"}}}),
          CreateCharacterToken(u8"\n            "),
          CreateStartTagToken({.Name = u8"link",
                               .Attributes = {{.Name = u8"type", .Value = u8"text/css"},
                                              {.Name = u8"href", .Value = u8"index.css"}}}),
          CreateCharacterToken(u8"\n            "),
          CreateStartTagToken({.Name = u8"title"}),
          CreateCharacterToken(u8"Document"),
          CreateEndTagToken({.Name = u8"title"}),
          CreateCharacterToken(u8"\n        "),
          CreateEndTagToken({.Name = u8"head"}),
          CreateCharacterToken(u8"\n        "),
          CreateStartTagToken({.Name = u8"body",
                               .Attributes = {{.Name = u8"style", .Value = u8"background-color: maroon;"}}}),
          CreateCharacterToken(u8"\n            "),
          CreateStartTagToken({.Name = u8"div",
                               .Attributes = {{.Name = u8"style",
                                               .Value = u8"\n              box-sizing: border-box;"}}}),
          CreateCharacterToken(u8"Hello, Krystal UI"),
          CreateEndTagToken({.Name = u8"div"}),
          CreateCharacterToken(u8"\n            "),
          CreateStartTagToken({.Name = u8"script",
                               .Attributes = {{.Name = u8"type", .Value = u8"javascript"},
                                              {.Name = u8"src", .Value = u8"index.js"}}}),
          CreateEndTagToken({.Name = u8"script"}),
          CreateCharacterToken(u8"\n        "),
          CreateEndTagToken({.Name = u8"body"}),
          CreateCharacterToken(u8"\n        "),
          CreateEndTagToken({.Name = u8"html"}),
          CreateEOFToken(),
        },
      .Errors = {},
    }));
}