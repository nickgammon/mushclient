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
        CleanupZstd();
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

// Handle TELOPT_COMPRESS4 subnegotiation with new protocol (the more future proof standard)
void CMUSHclientDoc::Handle_TELOPT_COMPRESS4()
{
    CString strMessage;  
    
    // Check if we have subnegotiation data
    if (m_IAC_subnegotiation_data.empty())
    {
        // Fallback mode: empty payload means "start zstd now" (backward compatibility)
        TRACE("MCCP4: Fallback mode - empty payload, starting zstd\n");
        m_MCCP4_active_encoding = "zstd";
        
        if (!InitZstd())
        {
            strMessage = Translate("Cannot initialize Zstandard decompression. World closed.");
            OnConnectionDisconnect();
            UMessageBox(strMessage, MB_ICONEXCLAMATION);
            return;
        }
        
        size_t const resetResult = ZSTD_DCtx_reset((ZSTD_DCtx*)m_zstd_dstream, ZSTD_reset_session_only);
        if (ZSTD_isError(resetResult))
        {
            strMessage = Translate("Cannot reset Zstandard decompression stream. World closed.");
            OnConnectionDisconnect();
            UMessageBox(strMessage, MB_ICONEXCLAMATION);
            return;
        }
        
        m_iMCCP_type = 4;
        m_bCompress = true;
        m_MCCP4_negotiation_active = false;
        return;
    }
    
    // Parse subnegotiation data
    unsigned char suboption = (unsigned char)m_IAC_subnegotiation_data[0];
    
    switch (suboption)
    {
        case MCCP4_BEGIN_ENCODING:
        {
            // Extract encoding name from data
            if (m_IAC_subnegotiation_data.size() < 2)
            {
                TRACE("MCCP4: Invalid BEGIN_ENCODING - no encoding name\n");
                Send_IAC_DONT(TELOPT_COMPRESS4);
                return;
            }
            
            string encoding = m_IAC_subnegotiation_data.substr(1);
            TRACE1("MCCP4: BEGIN_ENCODING %s\n", encoding.c_str());
            
            // Check if we support this encoding
            if (m_MCCP4_accepted_encodings.find(encoding) == string::npos)
            {
                strMessage.Format("MCCP4: Server chose unsupported encoding '%s'. Aborting compression.", encoding.c_str());
                TRACE1("%s\n", strMessage);
                Send_IAC_DONT(TELOPT_COMPRESS4);
                return;
            }
            
            // Initialize the appropriate codec
            if (encoding == "zstd")
            {
                if (!InitZstd())
                {
                    strMessage = Translate("Cannot initialize Zstandard decompression. World closed.");
                    OnConnectionDisconnect();
                    UMessageBox(strMessage, MB_ICONEXCLAMATION);
                    return;
                }
                
                size_t const resetResult = ZSTD_DCtx_reset((ZSTD_DCtx*)m_zstd_dstream, ZSTD_reset_session_only);
                if (ZSTD_isError(resetResult))
                {
                    strMessage = Translate("Cannot reset Zstandard decompression stream. World closed.");
                    OnConnectionDisconnect();
                    UMessageBox(strMessage, MB_ICONEXCLAMATION);
                    return;
                }
                m_iMCCP_type = 4;
            }
            else if (encoding == "deflate")
            {
                // Use existing MCCP2 deflate infrastructure
                if (!m_bCompressInitOK && !m_bCompress)
                    m_bCompressInitOK = InitZlib(m_zCompress);
                
                // Allocate compression buffers if not already allocated
                if (!m_CompressOutput)
                    m_CompressOutput = (Bytef *) malloc (m_nCompressionOutputBufferSize);
                if (!m_CompressInput)
                    m_CompressInput = (Bytef *) malloc (COMPRESS_BUFFER_LENGTH);
                
                if (!(m_bCompressInitOK && m_CompressOutput && m_CompressInput))
                {
                    strMessage = Translate("Cannot initialize deflate decompression. World closed.");
                    OnConnectionDisconnect();
                    UMessageBox(strMessage, MB_ICONEXCLAMATION);
                    return;
                }
                
                int izError = inflateReset(&m_zCompress);
                if (izError != Z_OK)
                {
                    strMessage = Translate("Cannot reset deflate decompression stream. World closed.");
                    OnConnectionDisconnect();
                    UMessageBox(strMessage, MB_ICONEXCLAMATION);
                    return;
                }
                m_iMCCP_type = 4;  // Still MCCP4, but using deflate
            }
            else
            {
                strMessage.Format("MCCP4: Unknown encoding '%s'. Aborting compression.", encoding.c_str());
                TRACE1("%s\n", strMessage);
                Send_IAC_DONT(TELOPT_COMPRESS4);
                return;
            }
            
            m_MCCP4_active_encoding = encoding;
            m_bCompress = true;
            m_MCCP4_negotiation_active = false;
            TRACE1("MCCP4: Compression active with encoding %s\n", encoding.c_str());
            break;
        }
        
        default:
        {
            // Unknown suboption - send WONT and continue (non-fatal)
            unsigned char wont_response[] = { IAC, SB, TELOPT_COMPRESS4, MCCP4_WONT, suboption, IAC, SE };
            SendPacket(wont_response, sizeof wont_response);
            TRACE1("MCCP4: Unknown suboption %d - sent WONT\n", (int)suboption);
            break;
        }
    }
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
            // Stop timing before error handling
            if (App.m_iCounterFrequency)
            {
                QueryPerformanceCounter(&finish);
                m_iCompressionTimeTaken += finish.QuadPart - start.QuadPart;
            }
            
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
                // Stop timing before error handling
                if (App.m_iCounterFrequency)
                {
                    QueryPerformanceCounter(&finish);
                    m_iCompressionTimeTaken += finish.QuadPart - start.QuadPart;
                }
                
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
        // Reset decoder state
        ZSTD_DCtx_reset((ZSTD_DCtx*)m_zstd_dstream, ZSTD_reset_session_only);
        
        // Check if remaining bytes in this buffer are raw telnet
        if (inBuf.pos < inBuf.size) 
        {
            unsigned char nextByte = ((unsigned char*)inBuf.src)[inBuf.pos];
            if (nextByte == 0xFF) // IAC - raw telnet follows in same buffer
            {
                // Disable compression so caller processes remaining bytes as raw telnet
                m_bCompress = false;
                m_iMCCP_type = 0;
                TRACE("MCCP4: Frame ended with IAC in buffer - disabled compression\n");
            }
            else
            {
                // More compressed data follows - keep compression active for concatenated frames
                TRACE("MCCP4: Frame ended with %d more bytes - keeping compression active for concatenated frames\n", 
                      (int)(inBuf.size - inBuf.pos));
                // Keep compression active and process remaining bytes as next frame
                // Don't return early - let normal flow handle it
            }
        }
        else
        {
            // No remaining bytes - keep compression active for next packet
            TRACE("MCCP4: Frame ended - keeping compression active for next packet\n");
        }
    }

    // Tell the caller how many INPUT bytes we consumed,
    // so it can immediately process any leftover (raw Telnet) bytes.
    return (int)inBuf.pos;
}