/* -------------- SHS.H --------------------------- */

typedef unsigned char BYTE;
typedef unsigned long SHSLONG;

#define SHS_BLOCKSIZE 64

#define SHS_DIGESTSIZE 20

typedef struct
  {
  SHSLONG digest [5];  // message digest
  SHSLONG countLo, countHi;  // 64-bit bit count
  SHSLONG data [16];   // SHS data buffer
  } SHS_INFO;

// include the define below if running on a little-endian machine, such as Intel

#define LITTLE_ENDIAN

  void shsInit (SHS_INFO * shsInfo);
  void shsUpdate (SHS_INFO * shsInfo, BYTE * buffer, int count);
  void shsFinal (SHS_INFO * shsInfo);
