/**
 * Objects of type ExtractorConfig store the necessary configuration to perform
 *   a stream extraction.
 */


#pragma once

#include <string>
using std::string;

#include <vector>
using std::vector;



struct ExtractorConfig {
public:
  // Path to input hevc bitstream
  string inputPath;

  // Path to output bitstream
  string outputPath;

  // Number of frames to skip before reaching desired random access point
  int numSkippedFrames;

  // Maximum temporal layer to be decoded
  int maxTemporalLayerId;



public:
  // Constructs with default configuration
  ExtractorConfig();

  // Constructs using configuration from command line arguments
  ExtractorConfig(int argc, char* argv[]);

  // Sets configuration using command line arguments
  bool loadFrom(int argc, char* argv[]);

  // Always include a virtual destructor
  virtual ~ExtractorConfig() = default;
};
