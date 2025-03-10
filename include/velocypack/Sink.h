////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2014-2020 ArangoDB GmbH, Cologne, Germany
/// Copyright 2004-2014 triAGENS GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is ArangoDB GmbH, Cologne, Germany
///
/// @author Max Neunhoeffer
/// @author Jan Steemann
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <fstream>
#include <sstream>

#include "velocypack/velocypack-common.h"
#include "velocypack/Buffer.h"

namespace arangodb::velocypack {

struct Sink {
  Sink() {}
  Sink(Sink const&) = delete;
  Sink& operator=(Sink const&) = delete;

  virtual ~Sink() = default;
  virtual void push_back(char c) = 0;
  virtual void append(std::string const& p) = 0;
  virtual void append(char const* p) = 0;
  virtual void append(char const* p, ValueLength len) = 0;
  virtual void reserve(ValueLength len) = 0;
};

template <typename T>
struct ByteBufferSinkImpl final : public Sink {
  explicit ByteBufferSinkImpl(Buffer<T>* buffer) : buffer(buffer) {}

  void push_back(char c) override final { buffer->push_back(c); }

  void append(std::string const& p) override final {
    buffer->append(p.c_str(), p.size());
  }

  void append(char const* p) override final { buffer->append(p, strlen(p)); }

  void append(char const* p, ValueLength len) override final {
    buffer->append(p, len);
  }

  void reserve(ValueLength len) override final { buffer->reserve(len); }

  Buffer<T>* buffer;
};

typedef ByteBufferSinkImpl<char> CharBufferSink;

template <typename T>
struct StringSinkImpl final : public Sink {
  explicit StringSinkImpl(T* buffer) : buffer(buffer) {}

  void push_back(char c) override final { buffer->push_back(c); }

  void append(std::string const& p) override final { buffer->append(p); }

  void append(char const* p) override final { buffer->append(p, strlen(p)); }

  void append(char const* p, ValueLength len) override final {
    buffer->append(p, checkOverflow(len));
  }

  void reserve(ValueLength len) override final {
    ValueLength length = len + buffer->size();
    if (length <= buffer->capacity()) {
      return;
    }
    buffer->reserve(checkOverflow(length));
  }

  T* buffer;
};

typedef StringSinkImpl<std::string> StringSink;

struct StringLengthSink final : public Sink {
  StringLengthSink() : length(0) {}

  void push_back(char) override final { ++length; }

  void append(std::string const& p) override final { length += p.size(); }

  void append(char const* p) override final { length += strlen(p); }

  void append(char const*, ValueLength len) override final {
    length += len;
  }

  void reserve(ValueLength) override final {}

  ValueLength length;
};

template <typename T>
struct StreamSinkImpl final : public Sink {
  explicit StreamSinkImpl(T* stream) : stream(stream) {}

  void push_back(char c) override final { *stream << c; }

  void append(std::string const& p) override final { *stream << p; }

  void append(char const* p) override final {
    stream->write(p, static_cast<std::streamsize>(strlen(p)));
  }

  void append(char const* p, ValueLength len) override final {
    stream->write(p, static_cast<std::streamsize>(len));
  }

  void reserve(ValueLength) override final {}

  T* stream;
};

typedef StreamSinkImpl<std::ostringstream> StringStreamSink;
typedef StreamSinkImpl<std::ofstream> OutputFileStreamSink;

}  // namespace arangodb::velocypack

using VPackSink = arangodb::velocypack::Sink;
using VPackCharBufferSink = arangodb::velocypack::CharBufferSink;
using VPackStringSink = arangodb::velocypack::StringSink;
using VPackStringLengthSink = arangodb::velocypack::StringLengthSink;
using VPackStringStreamSink = arangodb::velocypack::StringStreamSink;
using VPackOutputFileStreamSink = arangodb::velocypack::OutputFileStreamSink;
