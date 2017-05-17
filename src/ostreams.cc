// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/ostreams.h"
#include "src/objects.h"

#if V8_OS_WIN
#if _MSC_VER < 1900
#define snprintf sprintf_s
#endif
#elif V8_OS_ZOS
#include <unistd.h>
#endif

namespace v8 {
namespace internal {

OFStreamBase::OFStreamBase(FILE* f) : f_(f) {}


OFStreamBase::~OFStreamBase() {}


int OFStreamBase::sync() {
  std::fflush(f_);
  return 0;
}


OFStreamBase::int_type OFStreamBase::overflow(int_type c) {
  return (c != EOF) ? std::fputc(c, f_) : c;
}


std::streamsize OFStreamBase::xsputn(const char* s, std::streamsize n) {
#if V8_OS_ZOS
  if (f_ == stdout || f_ == stderr) {
    char str[n];
    memcpy(str, s, n);
    __a2e_l(&str[0], n);
    return static_cast<std::streamsize>(
        std::fwrite(str, 1, static_cast<size_t>(n), f_));
  }
  else
    return static_cast<std::streamsize>(
        std::fwrite(s, 1, static_cast<size_t>(n), f_));
#else
  return static_cast<std::streamsize>(
      std::fwrite(s, 1, static_cast<size_t>(n), f_));
#endif
}


OFStream::OFStream(FILE* f) : v8::base::OStream(nullptr), buf_(f) {
  DCHECK_NOT_NULL(f);
  rdbuf(&buf_);
}


OFStream::~OFStream() {}


namespace {

// Locale-independent predicates.
bool IsPrint(uint16_t c) { return 0x20 <= c && c <= 0x7e; }
bool IsSpace(uint16_t c) { return (0x9 <= c && c <= 0xd) || c == 0x20; }
bool IsOK(uint16_t c) { return (IsPrint(c) || IsSpace(c)) && c != '\\'; }


v8::base::OStream& PrintUC16(v8::base::OStream& os, uint16_t c, bool (*pred)(uint16_t)) {
  char buf[10];
  const char* format = pred(c) ? "%c" : (c <= 0xff) ? "\\x%02x" : "\\u%04x";
  snprintf(buf, sizeof(buf), format, c);
  return os << buf;
}


v8::base::OStream& PrintUC32(v8::base::OStream& os, int32_t c, bool (*pred)(uint16_t)) {
  if (c <= String::kMaxUtf16CodeUnit) {
    return PrintUC16(os, static_cast<uint16_t>(c), pred);
  }
  char buf[13];
  snprintf(buf, sizeof(buf), "\\u{%06x}", c);
  return os << buf;
}

}  // namespace


v8::base::OStream& operator<<(v8::base::OStream& os, const AsReversiblyEscapedUC16& c) {
  return PrintUC16(os, c.value, IsOK);
}


v8::base::OStream& operator<<(v8::base::OStream& os, const AsEscapedUC16ForJSON& c) {
  if (c.value == '\n') return os << "\\n";
  if (c.value == '\r') return os << "\\r";
  if (c.value == '\t') return os << "\\t";
  if (c.value == '\"') return os << "\\\"";
  return PrintUC16(os, c.value, IsOK);
}


v8::base::OStream& operator<<(v8::base::OStream& os, const AsUC16& c) {
  return PrintUC16(os, c.value, IsPrint);
}


v8::base::OStream& operator<<(v8::base::OStream& os, const AsUC32& c) {
  return PrintUC32(os, c.value, IsPrint);
}

}  // namespace internal
}  // namespace v8
