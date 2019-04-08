#pragma once

#include <vector>
using std::vector;


class ibitstream {
protected:
  const vector<uint8_t>* data;

  unsigned arrayIndex;

  unsigned numBitsRead;
  unsigned numHeldBits;
  unsigned char heldBits;


public:
  ibitstream();
  ibitstream(const vector<uint8_t>& data);

  void readFrom(const vector<uint8_t>& data);
  void readFrom(const vector<uint8_t>* data);

  unsigned read(unsigned numBits);

  void resetToBeginning();
};
