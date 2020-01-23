#ifndef WARPCOPYDISK_H
#define WARPCOPYDISK_H

#include "dimage.h"
#include "warpcopy.h"

#include <QObject>

//! a tool class to copy a whole disk via warp copy service
class WarpCopyDisk : public QThread
{
  Q_OBJECT
  
public:  
  enum Mode {
    READ_DISK_SLOW,
    WRITE_DISK_SLOW,
    READ_DISK_WARP,
    WRITE_DISK_WARP
  };
  
  WarpCopyDisk();
  ~WarpCopyDisk();
  
  //! start copy disk thread
  void startCopy(int drive,WarpCopyService *service,Mode mode,
                 const BlockMap &m_blockMap,QByteArray *m_rawImage);
  //! abort copy operation
  void abortCopy();
  //! is still copying?
  bool isCopying();
  //! after copying query errors and result
  bool finishCopy(int &numErrorBlocks);
  //! check for abort
  bool isAborted();

signals:
  //! report block
  void procBlock(int track,int sector,int status,const QString &statusString);
  //! prepare block
  void prepareBlock(int track,int sector);

protected slots:
  //! got a block
  void gotBlock(int drive,int track,int sector,int status,const QString &str,
                const QByteArray &block);
  //! put a block
  void putBlock(int drive,int track,int sector,int status,const QString &str);
  //! expect block
  void expectBlock(int drive,int track,int sector);
  //! finished warp operation
  void finishedWarpOp(bool ok);
  
protected:
  //! drive number
  int m_drive;
  //! warp copy service
  WarpCopyService *m_service;
  //! copy mode
  Mode m_mode;
  //! block map
  BlockMap m_blockMap;
  //! offset map
  OffsetMap m_offsetMap;
  //! image raw array
  QByteArray *m_rawImage;
  //! flags for block
  QVector<int> m_blockFlags;
  
  //! mutex for stop
  QMutex m_flagMutex;
  //! copier was aborted
  bool m_aborted;
  //! copier is running
  bool m_running;

  //! mutex for signal
  QMutex m_signalMutex;
  //! wait condidition for signal
  QWaitCondition m_signalWaitCond;
  //! block flag
  bool m_gotSignalFlag;
  //! signal per block?
  bool m_signalPerBlock;
  
  //! thread execution
  void run();

  // ----- copy modes -----
  //! read disk slow
  void readDiskSlow();
  //! write disk slow
  void writeDiskSlow();
  //! read disk warp
  void readDiskWarp();
  //! write disk warp
  void writeDiskWarp();
  //! get blocks for retry read - returns true if there are any
  bool getRetryBlocks(BlockPosVector &bpv);

  // ----- threading -----
  void resetSignal();
  void waitSignal();
  void wakeUpSignal();
};

#endif
