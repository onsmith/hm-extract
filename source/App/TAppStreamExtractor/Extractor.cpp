#include "Extractor.h"
#include "InputBitStream.h"
#include "NALUnit.h"

#include <fstream>
using std::ifstream;


void Extractor::run(const ExtractorConfig& config) {
  // The POC of the frame currently being decoded
  int pocBeingDecoded;

  // The POC of the frame last displayed
  int pocLastDisplayed = config.numSkippedFrames;

  // The decoded picture buffer
  //TComList<TComPic*>* dpb = NULL;

  // Open input file
  ifstream inputStream(config.inputPath, ifstream::in | ifstream::binary);
  if (!inputStream) {
    fprintf(
      stderr,
      "\nfailed to open bitstream file `%s' for reading\n",
      config.inputPath.c_str()
    );
    exit(EXIT_FAILURE);
  }
  InputBitStream inputBitStream(inputStream);

  // Main decoder loop
  while (!inputStream.fail()) {
    // Read next nal unit
    NALUnit nalu;
    nalu.readFrom(inputBitStream);

    // Check for an empty NAL unit
    /* this can happen if the following occur:
     *  - empty input file
     *  - two back-to-back start_code_prefixes
     *  - start_code_prefix immediately followed by EOF
     */
    if (nalu.isEmpty()) {
      fprintf(stderr, "Warning: Attempt to decode an empty NAL unit\n");


    } else {
      if ((m_iMaxTemporalLayer >= 0 && nalu.m_temporalId > m_iMaxTemporalLayer) || !isNaluWithinTargetDecLayerIdSet(&nalu)) {
        bNewPicture = false;
      } else {
        bNewPicture = m_cTDecTop.decode(nalu, m_iSkipFrame, m_iPOCLastDisplay);
        if (bNewPicture) {
          bitstreamFile.clear();
          /* location points to the current nalunit payload[1] due to the
           * need for the annexB parser to read three extra bytes.
           * [1] except for the first NAL unit in the file
           *     (but bNewPicture doesn't happen then) */
          bitstreamFile.seekg(location-streamoff(3));
          bytestream.reset();
        }
      }
    }

    if ( (bNewPicture || !bitstreamFile || nalu.m_nalUnitType == NAL_UNIT_EOS) &&
        !m_cTDecTop.getFirstSliceInSequence () )
    {
      if (!loopFiltered || bitstreamFile)
      {
        m_cTDecTop.executeLoopFilters(poc, pcListPic);
      }
      loopFiltered = (nalu.m_nalUnitType == NAL_UNIT_EOS);
      if (nalu.m_nalUnitType == NAL_UNIT_EOS)
      {
        m_cTDecTop.setFirstSliceInSequence(true);
      }
    }
    else if ( (bNewPicture || !bitstreamFile || nalu.m_nalUnitType == NAL_UNIT_EOS ) &&
              m_cTDecTop.getFirstSliceInSequence () ) 
    {
      m_cTDecTop.setFirstSliceInPicture (true);
    }

    if( pcListPic )
    {
      if ( (!m_reconFileName.empty()) && (!openedReconFile) )
      {
        const BitDepths &bitDepths=pcListPic->front()->getPicSym()->getSPS().getBitDepths(); // use bit depths of first reconstructed picture.
        for (Uint channelType = 0; channelType < MAX_NUM_CHANNEL_TYPE; channelType++)
        {
          if (m_outputBitDepth[channelType] == 0)
          {
            m_outputBitDepth[channelType] = bitDepths.recon[channelType];
          }
        }

        m_cTVideoIOYuvReconFile.open( m_reconFileName, true, m_outputBitDepth, m_outputBitDepth, bitDepths.recon ); // write mode
        openedReconFile = true;
      }
      // write reconstruction to file
      if( bNewPicture )
      {
        xWriteOutput( pcListPic, nalu.m_temporalId );
      }
      if ( (bNewPicture || nalu.m_nalUnitType == NAL_UNIT_CODED_SLICE_CRA) && m_cTDecTop.getNoOutputPriorPicsFlag() )
      {
        m_cTDecTop.checkNoOutputPriorPics( pcListPic );
        m_cTDecTop.setNoOutputPriorPicsFlag (false);
      }
      if ( bNewPicture &&
           (   nalu.m_nalUnitType == NAL_UNIT_CODED_SLICE_IDR_W_RADL
            || nalu.m_nalUnitType == NAL_UNIT_CODED_SLICE_IDR_N_LP
            || nalu.m_nalUnitType == NAL_UNIT_CODED_SLICE_BLA_N_LP
            || nalu.m_nalUnitType == NAL_UNIT_CODED_SLICE_BLA_W_RADL
            || nalu.m_nalUnitType == NAL_UNIT_CODED_SLICE_BLA_W_LP ) )
      {
        xFlushOutput( pcListPic );
      }
      if (nalu.m_nalUnitType == NAL_UNIT_EOS)
      {
        xWriteOutput( pcListPic, nalu.m_temporalId );
        m_cTDecTop.setFirstSliceInPicture (false);
      }
      // write reconstruction to file -- for additional bumping as defined in C.5.2.3
      if(!bNewPicture && nalu.m_nalUnitType >= NAL_UNIT_CODED_SLICE_TRAIL_N && nalu.m_nalUnitType <= NAL_UNIT_RESERVED_VCL31)
      {
        xWriteOutput( pcListPic, nalu.m_temporalId );
      }
    }
  }

  xFlushOutput( pcListPic );
  // delete buffers
  m_cTDecTop.deletePicBuffer();

  // destroy internal classes
  xDestroyDecLib();
}
