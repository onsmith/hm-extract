#include "InputBitStream.h"

#include <assert.h>


/**
  * Returns true if an EOF will be encountered within the next n bytes.
  */
bool InputBitStream::eofBeforeNBytes(unsigned n) {
  assert(n <= 4);
  if (m_NumFutureBytes >= n) {
    return false;
  }

  n -= m_NumFutureBytes;

  try {
    for (unsigned i = 0; i < n; i++) {
      m_FutureBytes = (m_FutureBytes << 8) | m_Input.get();
      m_NumFutureBytes++;
    }
  } catch (...) {
    return true;
  }
  return false;
}