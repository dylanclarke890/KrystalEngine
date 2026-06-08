#pragma once

#include "Krystal.HTML/Types/DOMString.hpp"
#include "Krystal.Lib/Core/Enum.hpp"
#include <cassert>

namespace Krys::HTML
{
  enum class ElementName : uint8
  {
    Unknown,
    a,
    abbr,
    acronym,
    address,
    applet,
    area,
    article,
    aside,
    audio,
    attachment,
    b,
    base,
    basefont,
    bdi,
    bdo,
    bgsound,
    big,
    blink,
    blockquote,
    body,
    br,
    button,
    canvas,
    caption,
    center,
    cite,
    code,
    col,
    colgroup,
    data,
    datalist,
    dd,
    del,
    details,
    dfn,
    dialog,
    dir,
    div,
    dl,
    dt,
    em,
    embed,
    fieldset,
    figcaption,
    figure,
    font,
    footer,
    form,
    frame,
    frameset,
    h1,
    h2,
    h3,
    h4,
    h5,
    h6,
    head,
    header,
    hgroup,
    hr,
    html,
    i,
    iframe,
    image,
    img,
    input,
    ins,
    isindex,
    kbd,
    keygen,
    label,
    legend,
    li,
    link,
    listing,
    main,
    map,
    mark,
    marquee,
    menu,
    meta,
    meter,
    model,
    multicol,
    nav,
    nextid,
    nobr,
    noembed,
    noframes,
    object,
    ol,
    optgroup,
    option,
    output,
    p,
    param,
    picture,
    plaintext,
    pre,
    progress,
    q,
    rb,
    rp,
    rt,
    rtc,
    ruby,
    s,
    samp,
    script,
    search,
    section,
    select,
    selectedcontent,
    slot,
    small,
    source,
    spacer,
    span,
    strike,
    strong,
    style,
    sub,
    summary,
    sup,
    table,
    tbody,
    td,
    template_, // conflicts with template keyword
    textarea,
    tfoot,
    th,
    thead,
    time,
    title,
    tr,
    track,
    tt,
    u,
    ul,
    var,
    video,
    wbr,
    xmp,
    noscript,
    color_profile,
    font_face,
    font_face_format,
    font_face_name,
    font_face_src,
    font_face_uri,
    missing_glyph,
    annotation_xml,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::ElementName, 153u);

namespace Krys::HTML
{
  KRYS_NODISCARD constexpr ElementName ParseElementName(DOMStringView tagName) noexcept
  {
    assert(!tagName.empty());

    switch (tagName.size())
    {
      case 1:
      {
        switch (tagName[0])
        {
          case u8'a':
          {
            return ElementName::a;
          }
          case u8'b':
          {
            return ElementName::b;
          }
          case u8'i':
          {
            return ElementName::i;
          }
          case u8'p':
          {
            return ElementName::p;
          }
          case u8'q':
          {
            return ElementName::q;
          }
          case u8's':
          {
            return ElementName::s;
          }
          case u8'u':
          {
            return ElementName::u;
          }
        }
        break;
      }
      case 2:
      {
        if (tagName == u8"br")
        {
          return ElementName::br;
        }
        if (tagName == u8"dd")
        {
          return ElementName::dd;
        }
        if (tagName == u8"dl")
        {
          return ElementName::dl;
        }
        if (tagName == u8"dt")
        {
          return ElementName::dt;
        }
        if (tagName == u8"em")
        {
          return ElementName::em;
        }
        if (tagName == u8"hr")
        {
          return ElementName::hr;
        }
        if (tagName == u8"h1")
        {
          return ElementName::h1;
        }
        if (tagName == u8"h2")
        {
          return ElementName::h2;
        }
        if (tagName == u8"h3")
        {
          return ElementName::h3;
        }
        if (tagName == u8"h4")
        {
          return ElementName::h4;
        }
        if (tagName == u8"h5")
        {
          return ElementName::h5;
        }
        if (tagName == u8"h6")
        {
          return ElementName::h6;
        }
        if (tagName == u8"li")
        {
          return ElementName::li;
        }
        if (tagName == u8"ol")
        {
          return ElementName::ol;
        }
        if (tagName == u8"ul")
        {
          return ElementName::ul;
        }
        if (tagName == u8"rb")
        {
          return ElementName::rb;
        }
        if (tagName == u8"rp")
        {
          return ElementName::rp;
        }
        if (tagName == u8"rt")
        {
          return ElementName::rt;
        }
        if (tagName == u8"td")
        {
          return ElementName::td;
        }
        if (tagName == u8"th")
        {
          return ElementName::th;
        }
        if (tagName == u8"tr")
        {
          return ElementName::tr;
        }
        if (tagName == u8"tt")
        {
          return ElementName::tt;
        }
        if (tagName == u8"ul")
        {
          return ElementName::ul;
        }
        break;
      }
      case 3:
      {
        if (tagName == u8"bdi")
        {
          return ElementName::bdi;
        }
        if (tagName == u8"bdo")
        {
          return ElementName::bdo;
        }
        if (tagName == u8"big")
        {
          return ElementName::big;
        }
        if (tagName == u8"col")
        {
          return ElementName::col;
        }
        if (tagName == u8"del")
        {
          return ElementName::del;
        }
        if (tagName == u8"dfn")
        {
          return ElementName::dfn;
        }
        if (tagName == u8"dir")
        {
          return ElementName::dir;
        }
        if (tagName == u8"div")
        {
          return ElementName::div;
        }
        if (tagName == u8"img")
        {
          return ElementName::img;
        }
        if (tagName == u8"ins")
        {
          return ElementName::ins;
        }
        if (tagName == u8"kbd")
        {
          return ElementName::kbd;
        }
        if (tagName == u8"map")
        {
          return ElementName::map;
        }
        if (tagName == u8"nav")
        {
          return ElementName::nav;
        }
        if (tagName == u8"pre")
        {
          return ElementName::pre;
        }
        if (tagName == u8"rtc")
        {
          return ElementName::rtc;
        }
        if (tagName == u8"sub")
        {
          return ElementName::sub;
        }
        if (tagName == u8"sup")
        {
          return ElementName::sup;
        }
        if (tagName == u8"var")
        {
          return ElementName::var;
        }
        if (tagName == u8"wbr")
        {
          return ElementName::wbr;
        }
        if (tagName == u8"xmp")
        {
          return ElementName::xmp;
        }
        break;
      }
      case 4:
      {
        if (tagName == u8"abbr")
        {
          return ElementName::abbr;
        }
        if (tagName == u8"area")
        {
          return ElementName::area;
        }
        if (tagName == u8"base")
        {
          return ElementName::base;
        }
        if (tagName == u8"body")
        {
          return ElementName::body;
        }
        if (tagName == u8"cite")
        {
          return ElementName::cite;
        }
        if (tagName == u8"code")
        {
          return ElementName::code;
        }
        if (tagName == u8"data")
        {
          return ElementName::data;
        }
        if (tagName == u8"font")
        {
          return ElementName::font;
        }
        if (tagName == u8"form")
        {
          return ElementName::form;
        }
        if (tagName == u8"head")
        {
          return ElementName::head;
        }
        if (tagName == u8"html")
        {
          return ElementName::html;
        }
        if (tagName == u8"link")
        {
          return ElementName::link;
        }
        if (tagName == u8"main")
        {
          return ElementName::main;
        }
        if (tagName == u8"mark")
        {
          return ElementName::mark;
        }
        if (tagName == u8"menu")
        {
          return ElementName::menu;
        }
        if (tagName == u8"meta")
        {
          return ElementName::meta;
        }
        if (tagName == u8"nobr")
        {
          return ElementName::nobr;
        }
        if (tagName == u8"ruby")
        {
          return ElementName::ruby;
        }
        if (tagName == u8"samp")
        {
          return ElementName::samp;
        }
        if (tagName == u8"slot")
        {
          return ElementName::slot;
        }
        if (tagName == u8"span")
        {
          return ElementName::span;
        }
        if (tagName == u8"time")
        {
          return ElementName::time;
        }
        break;
      }
      case 5:
      {
        if (tagName == u8"aside")
        {
          return ElementName::aside;
        }
        if (tagName == u8"audio")
        {
          return ElementName::audio;
        }
        if (tagName == u8"blink")
        {
          return ElementName::blink;
        }
        if (tagName == u8"embed")
        {
          return ElementName::embed;
        }
        if (tagName == u8"frame")
        {
          return ElementName::frame;
        }
        if (tagName == u8"image")
        {
          return ElementName::image;
        }
        if (tagName == u8"input")
        {
          return ElementName::input;
        }
        if (tagName == u8"label")
        {
          return ElementName::label;
        }
        if (tagName == u8"meter")
        {
          return ElementName::meter;
        }
        if (tagName == u8"model")
        {
          return ElementName::model;
        }
        if (tagName == u8"param")
        {
          return ElementName::param;
        }
        if (tagName == u8"small")
        {
          return ElementName::small;
        }
        if (tagName == u8"style")
        {
          return ElementName::style;
        }
        if (tagName == u8"table")
        {
          return ElementName::table;
        }
        if (tagName == u8"tbody")
        {
          return ElementName::tbody;
        }
        if (tagName == u8"tfoot")
        {
          return ElementName::tfoot;
        }
        if (tagName == u8"thead")
        {
          return ElementName::thead;
        }
        if (tagName == u8"title")
        {
          return ElementName::title;
        }
        if (tagName == u8"track")
        {
          return ElementName::track;
        }
        if (tagName == u8"video")
        {
          return ElementName::video;
        }
        break;
      }
      case 6:
      {
        if (tagName == u8"applet")
        {
          return ElementName::applet;
        }
        if (tagName == u8"button")
        {
          return ElementName::button;
        }
        if (tagName == u8"canvas")
        {
          return ElementName::canvas;
        }
        if (tagName == u8"center")
        {
          return ElementName::center;
        }
        if (tagName == u8"dialog")
        {
          return ElementName::dialog;
        }
        if (tagName == u8"figure")
        {
          return ElementName::figure;
        }
        if (tagName == u8"footer")
        {
          return ElementName::footer;
        }
        if (tagName == u8"header")
        {
          return ElementName::header;
        }
        if (tagName == u8"hgroup")
        {
          return ElementName::hgroup;
        }
        if (tagName == u8"iframe")
        {
          return ElementName::iframe;
        }
        if (tagName == u8"keygen")
        {
          return ElementName::keygen;
        }
        if (tagName == u8"legend")
        {
          return ElementName::legend;
        }
        if (tagName == u8"nextid")
        {
          return ElementName::nextid;
        }
        if (tagName == u8"object")
        {
          return ElementName::object;
        }
        if (tagName == u8"option")
        {
          return ElementName::option;
        }
        if (tagName == u8"output")
        {
          return ElementName::output;
        }
        if (tagName == u8"script")
        {
          return ElementName::script;
        }
        if (tagName == u8"search")
        {
          return ElementName::search;
        }
        if (tagName == u8"select")
        {
          return ElementName::select;
        }
        if (tagName == u8"source")
        {
          return ElementName::source;
        }
        if (tagName == u8"spacer")
        {
          return ElementName::spacer;
        }
        if (tagName == u8"strike")
        {
          return ElementName::strike;
        }
        if (tagName == u8"strong")
        {
          return ElementName::strong;
        }
        break;
      }
      case 7:
      {
        if (tagName == u8"acronym")
        {
          return ElementName::acronym;
        }
        if (tagName == u8"address")
        {
          return ElementName::address;
        }
        if (tagName == u8"article")
        {
          return ElementName::article;
        }
        if (tagName == u8"bgsound")
        {
          return ElementName::bgsound;
        }
        if (tagName == u8"caption")
        {
          return ElementName::caption;
        }
        if (tagName == u8"isindex")
        {
          return ElementName::isindex;
        }
        if (tagName == u8"listing")
        {
          return ElementName::listing;
        }
        if (tagName == u8"marquee")
        {
          return ElementName::marquee;
        }
        if (tagName == u8"noembed")
        {
          return ElementName::noembed;
        }
        if (tagName == u8"picture")
        {
          return ElementName::picture;
        }
        if (tagName == u8"section")
        {
          return ElementName::section;
        }
        if (tagName == u8"summary")
        {
          return ElementName::summary;
        }
        break;
      }
      case 8:
      {
        if (tagName == u8"basefont")
        {
          return ElementName::basefont;
        }
        if (tagName == u8"colgroup")
        {
          return ElementName::colgroup;
        }
        if (tagName == u8"datalist")
        {
          return ElementName::datalist;
        }
        if (tagName == u8"details")
        {
          return ElementName::details;
        }
        if (tagName == u8"fieldset")
        {
          return ElementName::fieldset;
        }
        if (tagName == u8"frameset")
        {
          return ElementName::frameset;
        }
        if (tagName == u8"multicol")
        {
          return ElementName::multicol;
        }
        if (tagName == u8"noframes")
        {
          return ElementName::noframes;
        }
        if (tagName == u8"optgroup")
        {
          return ElementName::optgroup;
        }
        if (tagName == u8"progress")
        {
          return ElementName::progress;
        }
        if (tagName == u8"template")
        {
          return ElementName::template_;
        }
        if (tagName == u8"textarea")
        {
          return ElementName::textarea;
        }
        if (tagName == u8"noscript")
        {
          return ElementName::noscript;
        }
      }
      case 9:
      {
        if (tagName == u8"plaintext")
        {
          return ElementName::plaintext;
        }
        if (tagName == u8"font-face")
        {
          return ElementName::font_face;
        }
        break;
      }
      case 10:
      {
        if (tagName == u8"attachment")
        {
          return ElementName::attachment;
        }
        if (tagName == u8"blockquote")
        {
          return ElementName::blockquote;
        }
        if (tagName == u8"figcaption")
        {
          return ElementName::figcaption;
        }
        break;
      }
      case 13:
      {
        if (tagName == u8"color-profile")
        {
          return ElementName::color_profile;
        }
        if (tagName == u8"font-face-src")
        {
          return ElementName::font_face_src;
        }
        if (tagName == u8"font-face-uri")
        {
          return ElementName::font_face_uri;
        }
        if (tagName == u8"missing-glyph")
        {
          return ElementName::missing_glyph;
        }
      }
      case 14:
      {
        if (tagName == u8"font-face-name")
        {
          return ElementName::font_face_name;
        }
        if (tagName == u8"annotation-xml")
        {
          return ElementName::annotation_xml;
        }
      }
      case 15:
      {
        if (tagName == u8"selectedcontent")
        {
          return ElementName::selectedcontent;
        }
      }
      case 16:
      {
        if (tagName == u8"font-face-format")
        {
          return ElementName::font_face_format;
        }
        break;
      }
    }

    return ElementName::Unknown;
  }
}