/**
 * Entry point/main function for stream extractor application.
 */


#include "Extractor.h"
#include "ExtractorConfig.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>



int main(int argc, char* argv[]) {
  // Create object to store command line configuration
  ExtractorConfig config;

  // Parse configuration
  if (!config.loadFrom(argc, argv)) {
    return EXIT_FAILURE;
  }

  // Get execution start time
  clock_t startTime = clock();

  // Run the extractor with the config object
  Extractor::run(config);

  // Get execution end time
  clock_t endTime = clock();

  // Calculate and report total execution time
  clock_t executionTime = (clock() - startTime) / CLOCKS_PER_SEC;
  printf("\n Total Time: %12.3f sec.\n", static_cast<double>(executionTime));

  return EXIT_SUCCESS;
}
