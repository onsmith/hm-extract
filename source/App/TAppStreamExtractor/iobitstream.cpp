#include "iobitstream.h"

#include <assert.h>


ibitstream::ibitstream() :
  data(nullptr),
  arrayIndex(0),
  numBitsRead(0),
  numHeldBits(0),
  heldBits(0) {
}


ibitstream::ibitstream(const vector<uint8_t>& data) :
  data(&data),
  arrayIndex(0),
  numBitsRead(0),
  numHeldBits(0),
  heldBits(0) {
}


void ibitstream::readFrom(const vector<uint8_t>& data) {
  this->data = &data;
  resetToBeginning();
}


void ibitstream::readFrom(const vector<uint8_t>* data) {
  this->data = data;
  resetToBeginning();
}


void ibitstream::resetToBeginning() {
  arrayIndex  = 0;
  numBitsRead = 0;
  numHeldBits = 0;
  heldBits    = 0;
}


unsigned ibitstream::read(unsigned numBits) {
  assert(numBits <= 32);
  auto& vec = *data;

  numBitsRead += numBits;

  /* NB, bits are extracted from the MSB of each byte. */
  unsigned retval = 0;
  if (numBits <= numHeldBits) {
    /* n=1, len(H)=7:   -VHH HHHH, shift_down=6, mask=0xfe
     * n=3, len(H)=7:   -VVV HHHH, shift_down=4, mask=0xf8
     */
    retval = heldBits >> (numHeldBits - numBits);
    retval &= ~(0xff << numBits);
    numHeldBits -= numBits;
    return retval;
  }

  /* all num_held_bits will go into retval
   *   => need to mask leftover bits from previous extractions
   *   => align retval with top of extracted word */
  /* n=5, len(H)=3: ---- -VVV, mask=0x07, shift_up=5-3=2,
   * n=9, len(H)=3: ---- -VVV, mask=0x07, shift_up=9-3=6 */
  numBits -= numHeldBits;
  retval = heldBits & ~(0xff << numHeldBits);
  retval <<= numBits;

  /* number of whole bytes that need to be loaded to form retval */
  /* n=32, len(H)=0, load 4bytes, shift_down=0
   * n=32, len(H)=1, load 4bytes, shift_down=1
   * n=31, len(H)=1, load 4bytes, shift_down=1+1
   * n=8,  len(H)=0, load 1byte,  shift_down=0
   * n=8,  len(H)=3, load 1byte,  shift_down=3
   * n=5,  len(H)=1, load 1byte,  shift_down=1+3
   */
  unsigned aligned_word = 0;
  unsigned num_bytes_to_load = (numBits - 1) >> 3;
  assert(arrayIndex + num_bytes_to_load < vec.size());

  switch (num_bytes_to_load)
  {
  case 3: aligned_word  = vec[arrayIndex++] << 24;
  case 2: aligned_word |= vec[arrayIndex++] << 16;
  case 1: aligned_word |= vec[arrayIndex++] <<  8;
  case 0: aligned_word |= vec[arrayIndex++];
  }

  /* resolve remainder bits */
  unsigned next_num_held_bits = (32 - numBits) % 8;

  /* copy required part of aligned_word into retval */
  retval |= aligned_word >> next_num_held_bits;

  /* store held bits */
  numHeldBits = next_num_held_bits;
  heldBits = aligned_word;

  return retval;
}
