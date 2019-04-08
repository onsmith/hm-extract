#include "ionalu.h"
using std::streampos;

#include <assert.h>



nalu::nalu(istream& input) {
  readFrom(input);
}



/**
 * Static function to peek at the next n bytes in an istream
 */
static uint32_t peekBytes(istream& input, unsigned n) {
  assert(n <= 4);
  uint32_t val = 0;
  streampos loc = input.tellg();
  for (int i = 0; i < n; i++) {
    val = (val << 8) | input.get();
  }
  input.seekg(loc);
  return val;
}



/**
 * Static function to read the next n bytes (n <= 4) from an istream
 */
static uint32_t readBytes(istream& input, unsigned n) {
  assert(n <= 4);
  uint32_t val = 0;
  for (int i = 0; i < n; i++) {
    val = (val << 8) | input.get();
  }
  return val;
}



/**
 * Static function to consume valid leading zero bytes from an istream
 */
static unsigned consumeLeadingZeroBytes(istream& input) {
  unsigned numLeadingZeroBytes = 0;

  while ((peekBytes(input, 3) != 0x000001   || input.eof())/* &&
         (peekBytes(input, 4) != 0x00000001 || input.eof())*/) {
    assert(input.get() == 0);
    numLeadingZeroBytes++;
  }

  return numLeadingZeroBytes;
}



/**
 * Static function to consume the start code from an istream
 */
static void consumeStartCode(istream& input) {
  uint32_t start_code_prefix_one_3bytes = readBytes(input, 3);
  assert(start_code_prefix_one_3bytes == 0x000001);
}



/**
 * Static function to read nal unit bytes from an istream into a vector
 */
static void readNaluBytes(istream& input, vector<uint8_t>& output) {
  while (peekBytes(input, 3) > 2 || input.eof()) {
    output.push_back(input.get());
  }
}



/**
 * Static function to consume valid trailing bytes from an istream
 */
static unsigned consumeTrailingZeroBytes(istream& input) {
  unsigned numTrailingZeroBytes = 0;

  while ((peekBytes(input, 3) != 0x000001   || input.eof()) &&
         (peekBytes(input, 4) != 0x00000001 || input.eof())) {
    uint8_t trailing_zero_8bits = input.get();
    assert(trailing_zero_8bits == 0);
    numTrailingZeroBytes++;
  }

  return numTrailingZeroBytes;
}



/**
 * Static function to remove emulation prevention bits from a data vector
 */
static void convertPayloadToRBSP(vector<uint8_t>& data, bool isVclNalUnit) {
  unsigned pos = 0;
  unsigned zeroCount = 0;
  vector<uint8_t>::iterator it_read, it_write;
  for (it_read = it_write = data.begin(); it_read != data.end(); it_read++, it_write++, pos++) {
    assert(zeroCount < 2 || *it_read >= 0x03);

    if (zeroCount == 2 && *it_read == 0x03) {
      pos++;
      it_read++;
      zeroCount = 0;

      if (it_read == data.end()) {
        break;
      }

      assert(*it_read <= 0x03);
    }

    if (*it_read == 0x00) {
      zeroCount++;
    }

    *it_write = *it_read;
  }

  assert(zeroCount == 0);

  if (isVclNalUnit) {
    // Remove cabac_zero_word from payload if present
    int n = 0;

    while (it_write[-1] == 0x00) {
      it_write--;
      n++;
    }

    if (n > 0) {
      printf("\nDetected %d instances of cabac_zero_word\n", n/2);
    }
  }

  data.resize(it_write - data.begin());
}



void nalu::readFrom(istream& input) {
  // Borrow the payload vector object
  vector<uint8_t>& data = payload;

  // Clear the vector in preparation for reading
  data.clear();

  // Consume leading zero bytes
  numLeadingZeroBytes = consumeLeadingZeroBytes(input);

  // Consume start code
  consumeStartCode(input);

  // Read raw bytes into the data vector
  readNaluBytes(input, data);

  // Consume trailing bytes
  numTrailingZeroBytes = consumeTrailingZeroBytes(input);

  // Parse the header at the beginning of the data vector
  ibitstream bs(data);
  readHeader(bs);

  // Perform emulation prevention
  convertPayloadToRBSP(data, isVCL());

  // Remove header bytes from payload vector
  payload.erase(payload.begin(), payload.begin() + 2);
}



void nalu::readHeader(ibitstream& data) {
  // Forbidden zero bit
  bool forbidden_zero_bit = data.read(1);
  assert(forbidden_zero_bit == 0);

  // Type
  type = static_cast<NALU_TYPE>(data.read(6));

  // Layer ID
  layerID = data.read(6);

  // Temporal ID
  temporalID = data.read(3) - 1;
}



bool nalu::isSlice() const {
  return type == NALU_TYPE::CODED_SLICE_TRAIL_R
      || type == NALU_TYPE::CODED_SLICE_TRAIL_N
      || type == NALU_TYPE::CODED_SLICE_TSA_R
      || type == NALU_TYPE::CODED_SLICE_TSA_N
      || type == NALU_TYPE::CODED_SLICE_STSA_R
      || type == NALU_TYPE::CODED_SLICE_STSA_N
      || type == NALU_TYPE::CODED_SLICE_BLA_W_LP
      || type == NALU_TYPE::CODED_SLICE_BLA_W_RADL
      || type == NALU_TYPE::CODED_SLICE_BLA_N_LP
      || type == NALU_TYPE::CODED_SLICE_IDR_W_RADL
      || type == NALU_TYPE::CODED_SLICE_IDR_N_LP
      || type == NALU_TYPE::CODED_SLICE_CRA
      || type == NALU_TYPE::CODED_SLICE_RADL_N
      || type == NALU_TYPE::CODED_SLICE_RADL_R
      || type == NALU_TYPE::CODED_SLICE_RASL_N
      || type == NALU_TYPE::CODED_SLICE_RASL_R;
}



bool nalu::isSEI() const {
  return type == NALU_TYPE::PREFIX_SEI
      || type == NALU_TYPE::SUFFIX_SEI;
}


bool nalu::isVCL() const {
  return (static_cast<unsigned>(type) < 32);
}
