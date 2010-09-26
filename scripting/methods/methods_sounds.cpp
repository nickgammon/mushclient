// methods_sounds.cpp

// Relating to playing sounds

#include "stdafx.h"
#include "..\..\MUSHclient.h"
#include "..\..\doc.h"
#include "..\errors.h"

// Implements:

//    GetSoundStatus
//    PlaySound
//    Sound
//    StopSound

// world.Sound - plays a sound

long CMUSHclientDoc::Sound(LPCTSTR SoundFileName) 
{
  if (SoundFileName [0] == 0)
    return eNoNameSpecified;

  if (!PlaySoundFile (SoundFileName))
  	return eCannotPlaySound;

  return eOK;
}   // end of CMUSHclientDoc::Sound


// start playing sound FileName in buffer Buffer (or in memory in MemoryBuffer for length MemLen)

// If not a fully-qualified path, prepend mushclient.exe directory followed by "sounds\"
// Loop when finished if Loop is true
// Play at volume 'Volume' (percent) - 0 is full volume, -100 is -100 db  
//   (3 db each time halves the volume, so -3 would be half volume, and -6 would be quarter volume)
// Pan at Pan -100 to +100 percent where -100 is fully left and +100 is fully right (and zero is centered)
// If Buffer is nonzero, and FileName is "" then adjust the volume, pan and loop flag for an existing, playing, sound, if possible
// If Buffer zero then try to take an unused buffer. If that fails, take a non-playing buffer. If that fails, take buffer 1
// Buffer should be zero (for any free buffer) or 1 to MAX_SOUND_BUFFERS (currently 10)

// Sound files must be .WAV files, 16-bit, 22.05KHz, PCM, Mono or stereo

// test: /print (PlaySound (9, "/Program Files/Mushclient/sounds/battle/battle01.wav", true, 100))

long CMUSHclientDoc::PlaySoundHelper(short Buffer, LPCTSTR FileName, BOOL Loop, double Volume, double Pan, LPCTSTR MemoryBuffer, size_t MemLen) 
{
int i;

  // no sound object? can't do it
  if (!App.m_pDirectSoundObject || !App.m_pDirectSoundPrimaryBuffer)
    return eCannotPlaySound ;

  // first release sound buffers for sounds that are not playing or are lost
  for (i = 0; i < MAX_SOUND_BUFFERS; i++)
    if (m_pDirectSoundSecondaryBuffer [i])
      {
      DWORD iStatus;
      if (SUCCEEDED (m_pDirectSoundSecondaryBuffer [i]->GetStatus (&iStatus)) &&
          (((iStatus & (DSBSTATUS_PLAYING | DSBSTATUS_LOOPING)) == 0) ||
          (iStatus & DSBSTATUS_BUFFERLOST ) ) )
        {  
        m_pDirectSoundSecondaryBuffer [i]->Release ();
        m_pDirectSoundSecondaryBuffer [i] = NULL;
        }
      }


  // -------------------------- convert volume from percent into db * 100 ----------------------------

  // Force volume into range
  if (Volume > 0 || Volume < (-100.0))
    Volume = 0.0;

  long iVolume = - (Volume / 100) * DSBVOLUME_MIN;


  // -------------------------- convert pan from percent into +/- db * 100 ----------------------------

  // Force Pan into range
  if (Pan > 100.0 || Pan < (-100))
    Pan = 0;

  long iPan = (Pan / 100) * DSBPAN_RIGHT;


  // ---------------------- if no filename, attempt to adjust existing sound ---------------------

  if (MemoryBuffer == NULL)
    {
    if (Buffer >= 1 && Buffer <= MAX_SOUND_BUFFERS && strlen (FileName) == 0)
      {
      Buffer--;   // make zero-relative

      // can't if not there
      if (m_pDirectSoundSecondaryBuffer [Buffer] == NULL)
        return eBadParameter;
      
      // don't if not playing
      DWORD iStatus;
      if (FAILED (m_pDirectSoundSecondaryBuffer [Buffer]->GetStatus (&iStatus)) ||
          (iStatus & DSBSTATUS_PLAYING) == 0)
        return eCannotPlaySound;

        // set requested panning
      m_pDirectSoundSecondaryBuffer [Buffer]->SetPan (iPan);
      // set requested volume
      m_pDirectSoundSecondaryBuffer [Buffer]->SetVolume (iVolume);
      // set requested looping
      m_pDirectSoundSecondaryBuffer [Buffer]->Play( 0, 0, Loop ? DSBPLAY_LOOPING : 0);

      return eOK;

      }

    // no file name AND no buffer number? can't do much
    if  (strlen (FileName) < 2)  // later I test the 1st and 2nd characters
      return eBadParameter;

    } // have a file name (not memory file)

  // ------------------- we have filename, get on with it -----------------------------

  // ---------------- if Buffer is zero, first try to find a free one -----------------

  if (Buffer == 0)
    {
    for (i = 0; i < (MAX_SOUND_BUFFERS / 2); i++)
      if (m_pDirectSoundSecondaryBuffer [i] == NULL)
        {
        Buffer = i + 1;
        break;
        } // end of found an unused slot
    }  // end of no buffer specifed

  // no free buffers? re-use buffer #1
  if (Buffer == 0)
    Buffer = 1;  

  // Make buffer zero-relative
  Buffer--;
  
  // Buffer must be in range
  if (Buffer < 0 || Buffer >= MAX_SOUND_BUFFERS)
    return eBadParameter;


  // ------------------- we have a buffer now -------------------------------

  CString strName;

  if (MemoryBuffer == NULL)
    {

    // ---------- if not absolute path, use MUSHclient.exe / sounds / filename 

    strName = FileName;

    strName.Replace ("/", "\\");  // convert forward to backslash

    // without a full pathname, assume in sounds directory under MUSHclient.exe
    if (!(
        isalpha ((unsigned char) strName [0]) &&
        strName [1] == ':'               // eg. c: blah
        ) &&      
        strName [0] != '\\')          // eg. \mydir\blah or \\server\blah
      strName = ExtractDirectory (App.m_strMUSHclientFileName) + "sounds\\" + strName;

    if (strName.GetLength () > 127)
      return eBadParameter;

    } // have a file name (not memory file)



  // ---------- open multimedia file ----------------------------

  HMMIO          mmioWave ;
  MMCKINFO       mmckinfoParent, mmckinfoSubchunk ;
  WAVEFORMATEX   wfPCM ;
  MMIOINFO mmioInfo;
  memset(&mmioInfo, 0, sizeof(MMIOINFO));

  // Open the wave data using the mm functions
  if (MemoryBuffer == NULL)
    {
    char sFilename [128];
    strcpy (sFilename, strName);  // make not const

    mmioWave = mmioOpen (sFilename, NULL, MMIO_READ);   // open from disk
    }   // have a file name (not memory file)
  else
    {  // open from memory
    mmioInfo.pchBuffer = (char *) MemoryBuffer;
    mmioInfo.cchBuffer = MemLen;
    mmioInfo.fccIOProc = FOURCC_MEM;
    mmioWave = mmioOpen (NULL, &mmioInfo, MMIO_READ) ;
    }

  if ( !mmioWave )
   return eFileNotFound;

  // now that we know we have a file there, release the sound buffer if possible

  // buffer currently in use? release it
  if (m_pDirectSoundSecondaryBuffer [Buffer])
    {
    DWORD iStatus;
    if (SUCCEEDED (m_pDirectSoundSecondaryBuffer [Buffer]->GetStatus (&iStatus)) &&
        (iStatus & DSBSTATUS_PLAYING))
      m_pDirectSoundSecondaryBuffer [Buffer]->Stop ();

    m_pDirectSoundSecondaryBuffer [Buffer]->Release ();
    m_pDirectSoundSecondaryBuffer [Buffer] = NULL;
    }

  // Descend to find a "WAVE" block, if this fails then the data is not
  // WAV data.
  mmckinfoParent.fccType = mmioFOURCC( 'W', 'A', 'V', 'E' ) ;
  if ( mmioDescend( mmioWave, &mmckinfoParent, NULL, MMIO_FINDRIFF ) ) 
   {
    mmioClose( mmioWave, 0 ) ;
    return eCannotPlaySound ;
   }

  // Descend again to the "fmt " block and retrieve the format information
  mmckinfoSubchunk.ckid = mmioFOURCC( 'f', 'm', 't', ' ' ) ;
  if ( mmioDescend( mmioWave, &mmckinfoSubchunk, &mmckinfoParent, 
                   MMIO_FINDCHUNK ) )
   {
    mmioClose( mmioWave, 0 ) ;
    return eCannotPlaySound ;
   }

  // This line actually reads the data from the "fmt " chunk, this data
  // should be in the form of a WAVEFORMATEX structure
  if ( mmioRead( mmioWave, (char *) &wfPCM, mmckinfoSubchunk.cksize ) == -1 )
   {
    mmioClose( mmioWave, 0 ) ;
    return eCannotPlaySound ;
   }

  // Step out a layer... think of the mm functions as step in and out of
  // hierarchies of "chunks" of information
  mmioAscend( mmioWave, &mmckinfoSubchunk, 0 ) ;

  // Find the "data" subchunk
  mmckinfoSubchunk.ckid = mmioFOURCC( 'd', 'a', 't', 'a' ) ;
  if ( mmioDescend( mmioWave, &mmckinfoSubchunk, &mmckinfoParent, 
                    MMIO_FINDCHUNK ) ) 
   {
    mmioClose( mmioWave, 0 ) ;
    return eCannotPlaySound ;
   }

 HRESULT        hr ;
 LPBYTE         lpvAudio1 ;
 LPBYTE         lpvAudio2 ;
 DWORD          dwWriteBytes1 ;
 DWORD          dwWriteBytes2 ;
 DSBUFFERDESC   bd ;   
 
 // At this point we have succeeded in finding the data for the WAV file so
 // we need to create a DirectSoundBuffer 
  // Set up bd structure for a static secondary buffer.       
  bd.dwSize = sizeof( bd ) ;       
  bd.dwFlags = (DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY) | DSBCAPS_STATIC;  

  // want to hear in background?
  if (m_bPlaySoundsInBackground)
    bd.dwFlags |= DSBCAPS_GLOBALFOCUS;

  // Buffer size retrieved from the mmckinfo structure for the data 
  // portion of the wav
  bd.dwBufferBytes = mmckinfoSubchunk.cksize ; 
  bd.dwReserved = 0 ;
  bd.lpwfxFormat = &wfPCM ; 

  // Create buffer.    
  if ( FAILED( App.m_pDirectSoundObject->CreateSoundBuffer( &bd, 
               &m_pDirectSoundSecondaryBuffer [Buffer], NULL ) ) )
   {
    mmioClose( mmioWave, 0 ) ;
    return eCannotPlaySound ;
   }
 
#define WRAP_UP_FAILED_SOUND \
    m_pDirectSoundSecondaryBuffer [Buffer]->Release ();  \
    m_pDirectSoundSecondaryBuffer [Buffer] = NULL;  \
    mmioClose( mmioWave, 0 ) ;  \
    return eCannotPlaySound;

 // make copy to save a bit of typing
 LPDIRECTSOUNDBUFFER lpdsBuffer = m_pDirectSoundSecondaryBuffer [Buffer];

 // Lock the buffer for the DirectSoundBuffer object
 hr = lpdsBuffer->Lock( 0, 0, (void **) &lpvAudio1, &dwWriteBytes1, 
               (void **) &lpvAudio2, &dwWriteBytes2, DSBLOCK_ENTIREBUFFER ) ;
 if ( hr == DSERR_BUFFERLOST )
   {
    // If the buffer was lost try restoring it and lock again
    lpdsBuffer->Restore() ;
    hr = lpdsBuffer->Lock( 0, 0, (void **) &lpvAudio1, &dwWriteBytes1, 
               (void **) &lpvAudio2, &dwWriteBytes2, DSBLOCK_ENTIREBUFFER ) ;
   }

  if ( FAILED( hr ) )
    {
    WRAP_UP_FAILED_SOUND;  // lock failed
    }
    
  if ( dwWriteBytes1 != mmckinfoSubchunk.cksize )
    {
    lpdsBuffer->Unlock( (void *) lpvAudio1, dwWriteBytes1, 
                        (void *) lpvAudio2, dwWriteBytes2 );
    WRAP_UP_FAILED_SOUND;  // wrong number of bytes
    }

  // Read the data directly into the locked buffer
  if ( mmioRead( mmioWave, ( char* ) lpvAudio1, mmckinfoSubchunk.cksize ) 
      == -1 )
    {
    lpdsBuffer->Unlock( (void *) lpvAudio1, dwWriteBytes1, 
                        (void *) lpvAudio2, dwWriteBytes2 );
    WRAP_UP_FAILED_SOUND;  // couldn't read
    }

  // Unlock the buffer
  if ( FAILED( lpdsBuffer->Unlock( (void *) lpvAudio1, dwWriteBytes1, 
                                  (void *) lpvAudio2, dwWriteBytes2 ) ) )
    {
    WRAP_UP_FAILED_SOUND;   // couldn't unlock
    }
 
  // Close the multimedia object
  mmioClose( mmioWave, 0 ) ;

  // set requested panning
  lpdsBuffer->SetPan (iPan);
  // set requested volume
  lpdsBuffer->SetVolume (iVolume);

 if (FAILED (lpdsBuffer->Play( 0, 0, Loop ? DSBPLAY_LOOPING : 0) ))
   {
    m_pDirectSoundSecondaryBuffer [Buffer]->Release ();
    m_pDirectSoundSecondaryBuffer [Buffer] = NULL;
    return eCannotPlaySound;
   }

  return eOK;
} // end of CMUSHclientDoc::PlaySoundHelper

long CMUSHclientDoc::PlaySound(short Buffer, LPCTSTR FileName, BOOL Loop, double Volume, double Pan) 
  {
  return PlaySoundHelper (Buffer, FileName, Loop, Volume, Pan, NULL, 0);
  }  // end of  CMUSHclientDoc::PlaySound


// stop the sound in the numbered Buffer from playing
// if Buffer is zero, stop all sounds
long CMUSHclientDoc::StopSound(short Buffer) 
{
  // buffer of zero means all
  if (Buffer == 0)
    {
    // stop sounds playing, release sound buffers
    for (int i = 0; i < MAX_SOUND_BUFFERS; i++)
      if (m_pDirectSoundSecondaryBuffer [i])
        {
        DWORD iStatus;
        if (SUCCEEDED (m_pDirectSoundSecondaryBuffer [i]->GetStatus (&iStatus)) &&
            (iStatus & DSBSTATUS_PLAYING))
          m_pDirectSoundSecondaryBuffer [i]->Stop ();

        m_pDirectSoundSecondaryBuffer [i]->Release ();
        m_pDirectSoundSecondaryBuffer [i] = NULL;

        }
      return eOK;
    }

  // Make buffer zero-relative
  Buffer--;
  
  // Buffer must be in range
  if (Buffer < 0 || Buffer >= MAX_SOUND_BUFFERS)
    return eBadParameter;

  // buffer currently in use? release it
  if (m_pDirectSoundSecondaryBuffer [Buffer])
    {
    DWORD iStatus;
    if (SUCCEEDED (m_pDirectSoundSecondaryBuffer [Buffer]->GetStatus (&iStatus)) &&
        (iStatus & DSBSTATUS_PLAYING))
      m_pDirectSoundSecondaryBuffer [Buffer]->Stop ();

    m_pDirectSoundSecondaryBuffer [Buffer]->Release ();
    m_pDirectSoundSecondaryBuffer [Buffer] = NULL;
    }

	return eOK;
}  // end of CMUSHclientDoc::StopSound

// set the status of the sound buffer
// -1 means buffer number out of range (1 to MAX_SOUND_BUFFERS)
// -2 means that the buffer is free
// -3 means sound system is not initialized (eg. Windows NT)
//  0 means the sound is NOT playing
//  1 means the sound is playing but not looping
//  2 means the sound is playing AND looping

long CMUSHclientDoc::GetSoundStatus(short Buffer) 
{

  // no sound object? can't do it
  if (!App.m_pDirectSoundObject || !App.m_pDirectSoundPrimaryBuffer)
    return -3;

  // Make buffer zero-relative
  Buffer--;
  
  // Buffer must be in range
  if (Buffer < 0 || Buffer >= MAX_SOUND_BUFFERS)
    return -1;

  if (!m_pDirectSoundSecondaryBuffer [Buffer])
    return -2;

  DWORD iStatus = 0;

  if (FAILED (m_pDirectSoundSecondaryBuffer [Buffer]->GetStatus (&iStatus)))
    return 0;  // can't get status, presume no sound

  if (iStatus & DSBSTATUS_PLAYING)
    return 1;

  if (iStatus & DSBSTATUS_LOOPING)
    return 2;

	return 0;  // not playing
}  // end of CMUSHclientDoc::GetSoundStatus
