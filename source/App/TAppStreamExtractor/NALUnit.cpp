#include "NALUnit.h"

#include <assert.h>


void NALUnit::readFrom(InputBitStream& input) {
  /* At the beginning of the decoding process, the decoder initialises its
   * current position in the byte stream to the beginning of the byte stream.
   * It then extracts and discards each leading_zero_8bits syntax element (if
   * present), moving the current position in the byte stream forward one
   * byte at a time, until the current position in the byte stream is such
   * that the next four bytes in the bitstream form the four-byte sequence
   * 0x00000001.
   */
  numLeadingZeroBytes = 0;
  while (
      (input.eofBeforeNBytes(24/8) || input.peekBytes(24/8) != 0x000001) &&
      (input.eofBeforeNBytes(32/8) || input.peekBytes(32/8) != 0x00000001)) {
    assert(bs.readByte() == 0);
    numLeadingZeroBytes++;
  }

  /* 1. When the next four bytes in the bitstream form the four-byte sequence
   * 0x00000001, the next byte in the byte stream (which is a zero_byte
   * syntax element) is extracted and discarded and the current position in
   * the byte stream is set equal to the position of the byte following this
   * discarded byte.
   */
  /* NB, the previous step guarantees this will succeed -- if EOF was
   * encountered, an exception will stop execution getting this far */
  if (bs.peekBytes(24/8) != 0x000001)
  {
    uint8_t zero_byte = bs.readByte();
    assert(zero_byte == 0);
    stats.m_numZeroByteBytes++;
  }

  /* 2. The next three-byte sequence in the byte stream (which is a
   * start_code_prefix_one_3bytes) is extracted and discarded and the current
   * position in the byte stream is set equal to the position of the byte
   * following this three-byte sequence.
   */
  /* NB, (1) guarantees that the next three bytes are 0x00 00 01 */
  assert(start_code_prefix_one_3bytes == 0x000001);
  stats.m_numStartCodePrefixBytes += 3;

  /* 3. NumBytesInNALunit is set equal to the number of bytes starting with
   * the byte at the current position in the byte stream up to and including
   * the last byte that precedes the location of any of the following
   * conditions:
   *   a. A subsequent byte-aligned three-byte sequence equal to 0x000000, or
   *   b. A subsequent byte-aligned three-byte sequence equal to 0x000001, or
   *   c. The end of the byte stream, as determined by unspecified means.
   */
  /* 4. NumBytesInNALunit bytes are removed from the bitstream and the
   * current position in the byte stream is advanced by NumBytesInNALunit
   * bytes. This sequence of bytes is nal_unit( NumBytesInNALunit ) and is
   * decoded using the NAL unit decoding process
   */
  /* NB, (unsigned)x > 2 implies n!=0 && n!=1 */
  while (bs.eofBeforeNBytes(24/8) || bs.peekBytes(24/8) > 2) {
    nalUnit.push_back(bs.readByte());
  }

  /* 5. When the current position in the byte stream is:
   *  - not at the end of the byte stream (as determined by unspecified means)
   *  - and the next bytes in the byte stream do not start with a three-byte
   *    sequence equal to 0x000001
   *  - and the next bytes in the byte stream do not start with a four byte
   *    sequence equal to 0x00000001,
   * the decoder extracts and discards each trailing_zero_8bits syntax
   * element, moving the current position in the byte stream forward one byte
   * at a time, until the current position in the byte stream is such that:
   *  - the next bytes in the byte stream form the four-byte sequence
   *    0x00000001 or
   *  - the end of the byte stream has been encountered (as determined by
   *    unspecified means).
   */
  /* NB, (3) guarantees there are at least three bytes available or none */
  while (
      (bs.eofBeforeNBytes(24/8) || bs.peekBytes(24/8) != 0x000001) &&
      (bs.eofBeforeNBytes(32/8) || bs.peekBytes(32/8) != 0x00000001)) {
    uint8_t trailing_zero_8bits = bs.readByte();
    assert(trailing_zero_8bits == 0);
    stats.m_numTrailingZero8BitsBytes++;
  }

  TComInputBitstream &bitstream = nalu.getBitstream();
  vector<uint8_t>& nalUnitBuf=bitstream.getFifo();
  // perform anti-emulation prevention
  convertPayloadToRBSP(nalUnitBuf, &bitstream, (nalUnitBuf[0] & 64) == 0);
  bitstream.resetToStart();
  readNalUnitHeader(nalu);
}
