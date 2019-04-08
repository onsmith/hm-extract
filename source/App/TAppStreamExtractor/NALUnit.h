#pragma once

#include "InputBitStream.h"


class NALUnit {
protected:
  int numLeadingZeroBytes;


public:
  NALUnit();
  NALUnit(InputBitStream& input);

  bool isEmpty() const;

  void readFrom(InputBitStream& input);
};
