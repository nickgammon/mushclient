/*
  MCCP4 (Zstandard) Support for MUSHclient
  -----------------------------------------
  
  This module implements MCCP4 using Zstandard compression, providing
  better compression ratios and performance compared to zlib-based MCCP1/2/3.

  Author: Jeffrey Johnston - asmodeusbrooding@gmail.com - https://github.com/Coffee-Nerd 
  MUSHclient MCCP4 Implementation based on MCCP4 specification using telnet option 88
*/

#include "stdafx.h"
#include "MUSHclient.h"
#include "doc.h"

// Initialize Zstandard decompression
bool CMUSHclientDoc::InitZstd()
{
    if (m_zstd_dstream)
        return true;  // Already initialized
    
    // Create decompression stream
    m_zstd_dstream = ZSTD_createDStream();
    if (!m_zstd_dstream)
        return false;
    
    // Initialize the stream
    size_t const initResult = ZSTD_initDStream((ZSTD_DStream*)m_zstd_dstream);
    if (ZSTD_isError(initResult))
    {
        ZSTD_freeDStream((ZSTD_DStream*)m_zstd_dstream);
        m_zstd_dstream = NULL;
        return false;
    }
    
    // Allocate buffers
    m_zstd_incap = ZSTD_DStreamInSize();
    m_zstd_outcap = ZSTD_DStreamOutSize();
    
    m_zstd_inbuf = (unsigned char*)malloc(m_zstd_incap);
    m_zstd_outbuf = (unsigned char*)malloc(m_zstd_outcap);
    
    if (!m_zstd_inbuf || !m_zstd_outbuf)
    {
        // Cleanup on allocation failure
        if (m_zstd_inbuf) free(m_zstd_inbuf);
        if (m_zstd_outbuf) free(m_zstd_outbuf);
        ZSTD_freeDStream((ZSTD_DStream*)m_zstd_dstream);
        m_zstd_dstream = NULL;
        m_zstd_inbuf = NULL;
        m_zstd_outbuf = NULL;
        m_zstd_incap = 0;
        m_zstd_outcap = 0;
        return false;
    }
    
    return true;
}

// Cleanup Zstandard decompression
void CMUSHclientDoc::CleanupZstd()
{
    if (m_zstd_dstream)
    {
        ZSTD_freeDStream((ZSTD_DStream*)m_zstd_dstream);
        m_zstd_dstream = NULL;
    }
    
    if (m_zstd_inbuf)
    {
        free(m_zstd_inbuf);
        m_zstd_inbuf = NULL;
    }
    
    if (m_zstd_outbuf)
    {
        free(m_zstd_outbuf);
        m_zstd_outbuf = NULL;
    }
    
    m_zstd_incap = 0;
    m_zstd_outcap = 0;
}

// Handle TELOPT_COMPRESS4 subnegotiation (like MCCP2)
void CMUSHclientDoc::Handle_TELOPT_COMPRESS4()
{
    CString strMessage;  
    // MCCP4: Received IAC SB COMPRESS4 IAC SE - starting compression
    m_iMCCP_type = 4;  // MCCP v4 (Zstandard)
    
    // Initialize compression library if not already done
    if (!InitZstd())
    {
        strMessage = Translate("Cannot initialize Zstandard decompression. World closed.");
        OnConnectionDisconnect();    // close the world
        UMessageBox(strMessage, MB_ICONEXCLAMATION);
        return;
    }
    
    // Reset the decompression stream for new data
    size_t const resetResult = ZSTD_DCtx_reset((ZSTD_DCtx*)m_zstd_dstream, ZSTD_reset_session_only);
    if (ZSTD_isError(resetResult))
    {
        strMessage = Translate("Cannot reset Zstandard decompression stream. World closed.");
        OnConnectionDisconnect();    // close the world
        UMessageBox(strMessage, MB_ICONEXCLAMATION);
        return;
    }

    m_bCompress = true;  // MCCP4: Zstandard decompression successfully activated
}

// Process Zstandard compressed data
int CMUSHclientDoc::ProcessZstdCompressed(const unsigned char* input, unsigned int inputSize)
{
    if (!m_zstd_dstream || !m_bCompress || m_iMCCP_type != 4)
        return 0;  // Not using Zstandard compression
    
    ZSTD_inBuffer inBuf = { input, (size_t)inputSize, 0 };
    size_t totalDecompressed = 0;
    bool frameEnded = false;

    // Collect decompressed output; do expensive work after timing
    unsigned char* collectedOutput = NULL;
    size_t collectedSize = 0;
    
    // Update compressed-bytes stats with input size (like MCCP2 does before timing)
    m_nTotalCompressed += inputSize;
    
    // Start timing ONLY the decompression (like MCCP2 does with inflate)
    LARGE_INTEGER start, finish;
    if (App.m_iCounterFrequency)
        QueryPerformanceCounter(&start);
    else
    {
        start.QuadPart = 0;
        finish.QuadPart = 0;
    }
    
    while (inBuf.pos < inBuf.size)
    {
        ZSTD_outBuffer outBuf = { m_zstd_outbuf, m_zstd_outcap, 0 };
        
        size_t const decompressResult = ZSTD_decompressStream((ZSTD_DStream*)m_zstd_dstream, &outBuf, &inBuf);
        
        if (ZSTD_isError(decompressResult))
        {
            // Decompression error
            CString strMessage;
            strMessage.Format(Translate("Zstandard decompression error: %s. World closed."), ZSTD_getErrorName(decompressResult));
            OnConnectionDisconnect();    // close the world
            UMessageBox(strMessage, MB_ICONEXCLAMATION);
            
            // Disable compression and cleanup
            m_bCompress = false;
            m_iMCCP_type = 0;
            CleanupZstd();
            if (collectedOutput) free(collectedOutput);
            return -1;
        }
        
        // Collect decompressed output (but don't display yet - we're timing)
        if (outBuf.pos > 0)
        {
            unsigned char* p = (unsigned char*)realloc(collectedOutput, collectedSize + outBuf.pos);
            if (!p)
            {
                if (collectedOutput) free(collectedOutput);
                // Memory allocation failure
                m_bCompress = false;
                m_iMCCP_type = 0;
                CleanupZstd();
                return -1;
            }
            collectedOutput = p;
            
            // Copy this chunk to collection buffer
            memcpy(collectedOutput + collectedSize, m_zstd_outbuf, outBuf.pos);
            collectedSize += outBuf.pos;
            
            // Update statistics
            m_nTotalUncompressed += outBuf.pos;
            totalDecompressed += outBuf.pos;
        }

        // r == 0 => end of current Zstd frame (we may still have trailing raw Telnet bytes)
        if (decompressResult == 0)
        {
            frameEnded = true;
            break; // leave loop; remaining bytes are NOT compressed
        }
    }
    
    // End timing here - after decompression but before I/O and display processing
    if (App.m_iCounterFrequency)
    {
        QueryPerformanceCounter(&finish);
        m_iCompressionTimeTaken += finish.QuadPart - start.QuadPart;
    }
    
    // Process the decompressed data through the normal display system (outside of timing)
    if (collectedSize > 0 && collectedOutput)
    {
        // Log raw data if enabled
        if (m_logfile && m_bLogRaw)
            WriteToLog((const char*)collectedOutput, (int)collectedSize);
        
        // Process the decompressed data through the normal display system
        DisplayMsg((const char*)collectedOutput, (int)collectedSize, 0);
        
        // Free the collection buffer
        free(collectedOutput);
    }

    if (frameEnded)
    {
        // Reset DCtx for potential future COMPRESS4 start; exit MCCP mode now.
        ZSTD_DCtx_reset((ZSTD_DCtx*)m_zstd_dstream, ZSTD_reset_session_only);
        m_bCompress = false;
        m_iMCCP_type = 0;
    }

    // Tell the caller how many INPUT bytes we consumed,
    // so it can immediately process any leftover (raw Telnet) bytes.
    return (int)inBuf.pos;
}