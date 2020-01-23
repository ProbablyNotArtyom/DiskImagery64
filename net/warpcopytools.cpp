#include "warpcopytools.h"

WarpCopyTools::WarpCopyTools()
{
  m_gcrBlock.resize(325);
  m_dataBlock.resize(260);
}

WarpCopyTools::~WarpCopyTools()
{
}

// ---------- encode --------------------------------------------------------

void WarpCopyTools::encodeBlock(const QByteArray &realBlock,QByteArray &rawBlock,
                                int track,int sector)
{
  // build 260 bytes DOS block
  m_dataBlock[0] = 0x07; // CBM Dos Data Block Marker
  char checksum = 0;
  for(int i=0;i<256;i++) {
    char value = realBlock[i];
    m_dataBlock[i+1] = value;
    checksum ^= value;
  }
  m_dataBlock[257] = checksum;
  m_dataBlock[258] = 0;
  m_dataBlock[259] = 0;
  
  // encode 260 bytes data into 325 gcr bytes
  const char *dataPtr = m_dataBlock.constData();
  char *gcrPtr = m_gcrBlock.data();
  for(int i=0;i<65;i++) {
    encode4DataTo5GCR(dataPtr,gcrPtr);
    dataPtr+=4;
    gcrPtr+=5;
  }

  // move one gcr byte to the left 
  // (throw away first gcr byte of encoded 0x07 (CBM Data Block) = always 0x55)
  // (throw away last 2 gcr bytes of encoded 0x00 0x00 = always 0x29 0x4a)
  // -> 322 gcr bytes left
  for(int i=0;i<322;i++) {
    char value = m_gcrBlock[i+1];
    rawBlock[i] = value;
  }
  
  // fill in data
  rawBlock[322] = sector;
  rawBlock[323] = 0x71; // ??
  rawBlock[324] = track;
  rawBlock[325] = 0;
  rawBlock[326] = 0xb2; // ??
  rawBlock[327] = 0x0d; //`??
  
#if 0
  // dump block
  for(int i=0;i<328;i++) {
    if(i%16==0)
      printf("%04x: ",i);
    printf("%02x ",(char)rawBlock[i]&0xff);
    if(i%16==15)
      printf("\n");
  }
  printf("\n");
#endif
}

void WarpCopyTools::encode4DataTo5GCR(const char *data,char *gcr)
{
  char d[8];
  char g[8];
  d[0] = (data[0]>>4) & 0xf;
  d[1] =  data[0] & 0xf;
  d[2] = (data[1]>>4) & 0xf;
  d[3] =  data[1] & 0xf;
  d[4] = (data[2]>>4) & 0xf;
  d[5] =  data[2] & 0xf;
  d[6] = (data[3]>>4) & 0xf;
  d[7] =  data[3] & 0xf;
  
  for(int i=0;i<8;i++) {
    encodeNybbleToGCR(d[i],g[i]);
  }

  gcr[0] = ((g[0] << 3) & 0xf8) | ((g[1] >> 2) & 0x07);                        // 5+3
  gcr[1] = ((g[1] << 6) & 0xc0) | ((g[2] << 1) & 0x3e) | ((g[3] >> 4) & 0x01); // 2+5+1
  gcr[2] = ((g[3] << 4) & 0xf0) | ((g[4] >> 1) & 0x0f);                        // 4+4
  gcr[3] = ((g[4] << 7) & 0x80) | ((g[5] << 2) & 0x7c) | ((g[6] >> 3) & 0x03); // 1+5+2
  gcr[4] = ((g[6] << 5) & 0xe0) | ( g[7]       & 0x1f);                        // 3+5
}

void WarpCopyTools::encodeNybbleToGCR(const char &data,char &gcr)
{
  const char gcrCode[]={10,11,18,19,14,15,22,23,9,25,26,27,13,29,30,21};
  gcr = gcrCode[data&0xf];
}

// ---------- decode --------------------------------------------------------

int WarpCopyTools::decodeBlock(const QByteArray &rawBlock,
                               QByteArray &realBlock,
                               int &track,int &sector,bool decodeSector)
{
  char checksum = rawBlock[323];
  track = rawBlock[324];
  // char sectorNum = rawBlock[325]; sector counter per track
  sector = -1; // mark initially as invalid

  // server error?
  if(rawBlock[321]==(char)0xf0)
    return WCB_HARD_ERROR;

  // check packet checksum
  char mycheck = calcChecksum(rawBlock);
  if(mycheck!=checksum)
    return WCB_NET_CHECKSUM_ERROR;
  
  // transform block
  char gcrSector;
  bool lastSector;
  unpackBlock(rawBlock,m_gcrBlock,gcrSector,lastSector);

  // decode sector number
  if(decodeSector) {
    char nybSector;
    if(!decodeGCRtoNybble(gcrSector,nybSector))
      return WCB_GCR_HEADER_ERROR;
    sector = nybSector;
    // use lowest gcr bit of 2nd nibble to decide if sector>15
    if(gcrSector & 0x20)
      sector += 0x10;
  } else {
    sector = -1;
  }
  
  // decode block
  const char *gcrPtr = m_gcrBlock.constData();
  char *dataPtr = m_dataBlock.data(); 
  for(int i=0;i<65;i++) {
    if(!decode5GCRto4Data(gcrPtr,dataPtr)) {
      return WCB_GCR_DATA_ERROR;
    }
    gcrPtr+=5;
    dataPtr+=4;
  }
  
  // copy block and check DOS block checksum
  checksum = 0;
  for(int i=0;i<256;i++) {
    char c = m_dataBlock[i+1];
    realBlock[i] = c;
    checksum ^= c;
  }
  char blockChecksum=m_dataBlock[257];
  if(checksum!=blockChecksum)
    return WCB_BLOCK_CHECKSUM_ERROR;
  
  return WCB_OK;
}

char WarpCopyTools::calcChecksum(const QByteArray &rawBlock)
{
  char checksum = 0;
  for(int i=0;i<323;i++) {
    if(i==321) {
      checksum ^= rawBlock[i] | 0x08; // mask out flags
    }
    else
      checksum ^= rawBlock[i];
  }
  return checksum;
}

// transformBlock - see mwc's DiskReader.cpp for details!
void WarpCopyTools::unpackBlock(const QByteArray &rawBlock,
                                QByteArray &gcrBlock,
                                char &gcrSector,
                                bool &lastSector)
{
  const char nybbleTrafo[16] = {
    0xf,0x7,0xd,0x5, 0xb,0x3,0x9,0x1,
    0xe,0x6,0xc,0x4, 0xa,0x2,0x8,0x0
  };

  const char *raw = rawBlock.constData();
  gcrBlock[0] = 0x55; // start gcr of data block marker $07
  char *trafo = gcrBlock.data()+1;
  for(int i=0;i<323;i++) {    
    char in = *(raw++);
    int n1 = (in & 0xf);
    int n2 = (in>>4) & 0xf;
    *(trafo++) = (nybbleTrafo[n2]<<4) | nybbleTrafo[n1];
  }
  
  // extract overwritten data
  lastSector = gcrBlock[322] & 1;
  gcrSector  = gcrBlock[323];
  
  // reconstruct gcr data: two 0 bytes = 4 nybbles -> 20 bit gcr data
  gcrBlock[322] = (gcrBlock[322] & 0xf0) | 0x05;
  gcrBlock[323] = 0x29;
  gcrBlock[324] = 0x4a; // was checksum!
}

bool WarpCopyTools::decodeGCRtoNybble(const char gcr,char &nybble)
{
  const char gcrCode[]={10,11,18,19,14,15,22,23,9,25,26,27,13,29,30,21};
  char gcr2 = gcr & 0x1f;
  for(int i=0;i<16;i++) {
    if(gcr2==gcrCode[i]) {
      nybble = i;
      return true;
    }
  }
  return false;
}

bool WarpCopyTools::decode5GCRto4Data(const char *gcr,char *data)
{
  char n[8];
  char d[8];
  n[0]=(gcr[0]>>3)&31;
  n[1]=((((gcr[0]&7)<<2)&28) | ((gcr[1]>>6)&3));
  n[2]=((gcr[1]>>1)&31);
  n[3]=(((gcr[1]<<4)&31) | ((gcr[2]>>4)&15));
  n[4]=(((gcr[2]<<1)&31) | ((gcr[3]>>7)&1));
  n[5]=((gcr[3]>>2)&31);
  n[6]=(((gcr[3]<<3)&31) | ((gcr[4]>>5)&7));
  n[7]=(gcr[4]&31);

  bool ok = true;
  for (int i=0;i<8;i++) {
    if(!decodeGCRtoNybble(n[i],d[i])) {
      ok = false;
    }
  }

  data[0]=(d[0]<<4)+(d[1]&0x0f);
  data[1]=(d[2]<<4)+(d[3]&0x0f);
  data[2]=(d[4]<<4)+(d[5]&0x0f);
  data[3]=(d[6]<<4)+(d[7]&0x0f);
  return ok;
}

