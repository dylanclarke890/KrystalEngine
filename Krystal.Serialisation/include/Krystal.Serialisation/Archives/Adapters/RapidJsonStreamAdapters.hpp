#pragma once

#include "Krystal.IO/Streams/Stream.hpp"
#include "Krystal.Lib/Mixins/NonCopyMovable.hpp"
#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include <cassert>

namespace Krys::Serialisation
{
  class RapidJsonStreamWriterAdapter : NonCopyMovable<RapidJsonStreamWriterAdapter>
  {
  private:
    IO::IStreamWriter &_stream;

  public:
    typedef char Ch;

    RapidJsonStreamWriterAdapter(IO::IStreamWriter &stream) : _stream(stream)
    {
    }

    void Put(Ch c)
    {
      Array<byte, 1> data = {static_cast<byte>(c)};
      _stream.Write(data);
    }

    Ch *PutBegin()
    {
      assert(false && "Not implemented!");
      return 0;
    }

    size_t PutEnd(Ch *)
    {
      assert(false && "Not implemented!");
      return 0;
    }

    void Flush()
    {
      _stream.Flush();
    }
  };

  class RapidJsonStreamReaderAdapter : NonCopyMovable<RapidJsonStreamReaderAdapter>
  {
  private:
    IO::IStreamReader &_stream;

  public:
    typedef char Ch;

    RapidJsonStreamReaderAdapter(IO::IStreamReader &stream) : _stream(stream)
    {
    }

    Ch Peek() const
    {
      if (byte next; _stream.Peek(next))
      {
        return (Ch)next;
      }
      return '\0';
    }

    Ch Take()
    {
      Array<byte, 1> next {};
      if (_stream.Read(next) == 1)
      {
        return static_cast<Ch>(next[0]);
      }
      return '\0';
    }

    size_t Tell() const
    {
      return (size_t)_stream.Position();
    }

    void Put(Ch)
    {
      assert(false && "Not implemented!");
    }

    Ch *PutBegin()
    {
      assert(false && "Not implemented!");
      return 0;
    }

    size_t PutEnd(Ch *)
    {
      assert(false && "Not implemented!");
      return 0;
    }
  };
}