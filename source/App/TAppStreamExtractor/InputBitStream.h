#pragma once

#include <string>
using std::string;

#include <iostream>
using std::istream;


class InputBitStream {
protected:
  istream* stream;


public:
  InputBitStream();
  InputBitStream(istream* stream);
  InputBitStream(istream& stream);

  void open(istream* stream);
  void open(istream& stream);

  bool eofBeforeNBytes(unsigned n) const;

  ~InputBitStream() = default;
};
