#ifndef WARPCOPYTOOLS_H
#define WARPCOPYTOOLS_H

#include <QtCore/QtCore>

class BlockPos {
public:
  BlockPos()
  : m_track(0),m_sector(0) {}
  BlockPos(int t,int s)
  : m_track(t),m_sector(s) {}
  int track() const { return m_track; }
  int sector() const { return m_sector; }
protected:
  int m_track;
  int m_sector;
};

typedef QVector<BlockPos> BlockPosVector;

enum WarpCopyBlockStatus {
  WCB_OK=0,
  WCB_NET_ERROR=1,
  WCB_NET_CHECKSUM_ERROR=2,
  WCB_GCR_HEADER_ERROR=3,
  WCB_GCR_DATA_ERROR=4,
  WCB_BLOCK_CHECKSUM_ERROR=5,
  WCB_HARD_ERROR=6,
  WCB_MISSING=99
};

class WarpCopyTools
{
public:
  WarpCopyTools();
  ~WarpCopyTools();

  //! decode raw 328 byte warpcopy block into 256 bytes real block data
  int decodeBlock(const QByteArray &rawBlock,QByteArray &realBlock,
                  int &track,int &sector,bool decodeSector);
                  
  //! encode a 256 byte real block intor a 328 byte warpcopy block
  void encodeBlock(const QByteArray &realBlock,QByteArray &rawBlock,
                   int track,int sector);

protected:
  QByteArray m_gcrBlock;
  QByteArray m_dataBlock;
  
  //! unpack block
  void unpackBlock(const QByteArray &rawBlock,QByteArray &gcrBlock,
                   char &gcrSector,bool &lastSector);
  //! calc checksum
  char calcChecksum(const QByteArray &rawBlock);
  //! decode 5bit GCR to 4bit data
  bool decodeGCRtoNybble(const char gcr,char &nybble);  
  //! decode 5 GCR bytes to 4 data bytes
  bool decode5GCRto4Data(const char *gcr,char *data);
  
  //! encode 4 data bytes to 5 GCR bytes
  void encode4DataTo5GCR(const char *data,char *gcr);
  //! encode 4bit data to 5bit GCR
  void encodeNybbleToGCR(const char &nybble,char &gcr);
};

#endif

