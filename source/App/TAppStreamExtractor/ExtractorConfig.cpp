#include "ExtractorConfig.h"

#include <iostream>
using std::cout;

#include <list>
using std::list;

#include "Lib/TAppCommon/program_options_lite.h"
using df::program_options_lite::Options;
using df::program_options_lite::setDefaults;
using df::program_options_lite::ErrorReporter;
using df::program_options_lite::doHelp;
using df::program_options_lite::scanArgv;



// Constructs with default configuration
ExtractorConfig::ExtractorConfig() :
  inputPath(),
  outputPath(),
  numSkippedFrames(0),
  maxTemporalLayerId(-1) {
}



// Constructs using configuration from command line arguments
ExtractorConfig::ExtractorConfig(int argc, char* argv[]) :
  inputPath(),
  outputPath(),
  numSkippedFrames(0),
  maxTemporalLayerId(-1) {
  loadFrom(argc, argv);
}



// Sets configuration using command line arguments
bool ExtractorConfig::loadFrom(int argc, char* argv[]) {
  bool willShowHelp;
  bool willWarnUnknownParams;

  // List accepted options
  Options opts;
  opts.addOptions()
  ("help,h",   willShowHelp,          false,      "Display this help text")
  ("input,i",  inputPath,         string(""), "Path to input hevc bitstream")
  ("output,o", outputPath,            string(""), "Path to output bitstream")
  ("warn,w",   willWarnUnknownParams, false,      "Warn for unknown configuration parameters instead of failing");
  //("seek,s",   numSkippedFrames,      0,          "Number of frames to skip before random access")
  //("layer,l",  maxTemporalLayerId,    -1,         "Maximum temporal layer id to be decoded, where a value of -1 decodes all layers");

  // Set default options
  setDefaults(opts);

  // Parse command line arguments
  ErrorReporter err;
  const list<const char*>& argv_unhandled = scanArgv(opts, argc, (const char**) argv, err);
  for (list<const char*>::const_iterator it = argv_unhandled.begin(); it != argv_unhandled.end(); it++) {
    fprintf(stderr, "Unhandled argument ignored: `%s'\n", *it);
  }

  // Handle printing help message
  if (argc == 1 || willShowHelp) {
    doHelp(cout, opts);
    return false;
  }

  // Handle unknown parameters
  if (err.is_errored && !willWarnUnknownParams) {
    return false;
  }

  // Input bitstream path is required
  if (inputPath.empty()) {
    fprintf(stderr, "No input file specified, aborting\n");
    return false;
  }

   // Output path is required
  if (outputPath.empty()) {
    fprintf(stderr, "No output file specified, aborting\n");
    return false;
  }

  return true;
}
