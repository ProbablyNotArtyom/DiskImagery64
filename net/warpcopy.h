#ifndef WARPCOPY_H
#define WARPCOPY_H

#include <QtNetwork/QtNetwork>
#include "netservice.h"
#include "cbmfile.h"
#include "dimage.h"
#include "warpcopytools.h"

#include <QObject>

class WarpCopyTask;

class WarpCopyService : public NetService
{
  Q_OBJECT

public:
  WarpCopyService();
  ~WarpCopyService();

  // ----- warp copy services -----
  //! send a DOS command
  WarpCopyTask *sendDOSCommand(int drive,const QString &command);
  //! get the drive status - emit gotDriveStatus
  WarpCopyTask *getDriveStatus(int drive);
  //! get the directory - emits gotDirectory
  WarpCopyTask *getDirectory(int drive);

  //! read disk block (slow) - emits gotBlock
  WarpCopyTask *readBlock(int drive,int track,int sector);
  //! write disk block (slow) - emits putBlock
  WarpCopyTask *writeBlock(int drive,int track,int sector,const QByteArray &block);

  //! raw read a set of track/sectors - emits n gotBlock + finishedWarpOp
  WarpCopyTask *warpReadBlocks(int drive,const BlockPosVector &ts);
  //! warp read disk - emits n gotBlock + finishedWarpOp
  WarpCopyTask *warpReadDisk(int drive,int numBlocks);

  //! warp write a disk - emits n putBlock + finishedWarpOp
  WarpCopyTask *warpWriteDisk(int drive,const BlockMap &bm,const QByteArray &image);

signals:
  //! got a drive status
  void gotDriveStatus(int drive,const QString &status);
  //! got a directory
  void gotDirectory(int drive,const CBMFileList &dir);
  //! got a block
  void gotBlock(int drive,int track,int sector,int status,const QString &statusStr,
                const QByteArray &block);
  //! put a block
  void putBlock(int drive,int track,int sector,int status,const QString &statusStr);
  //! expect a block
  void expectBlock(int drive,int track,int sector);
  //! finished warp op
  void finishedWarpOp(bool ok);

protected:
  WarpCopyTools m_tools;

  //! handle finished task
  void finishedTask(NetTask *task);
  //! handle keep task
  void keepTask(NetTask *task);

  //! convert image to raw image
  void convertImageToRawImage(const BlockMap &bm,
                              const QByteArray &image,
                              QByteArray &rawImage);

  //! return status string
  QString statusString(int status) const;
  //! full status string
  QString fullStatusString(int track,int sector,int status) const;
};

class WarpCopyTask : public NetTask
{
public:
  enum Job {
    EXEC_COMMAND,
    READ_DIRECTORY,
    READ_STATUS,
    READ_BLOCK,
    WRITE_BLOCK,
    WARP_READ_BLOCKS,
    WARP_READ_DISK,
    WARP_WRITE_DISK
  };

  //! create a task with given job on the drive
  WarpCopyTask(Job job,int drive);
  //! remove task
  ~WarpCopyTask();

  //! return the job of this task
  Job job() const { return m_job; }
  //! return the drive
  int drive() const { return m_drive; }

  // main run in task
  virtual Result run(NetHost &host);

  //! store job data
  void setJobData(const QByteArray &jobData) { m_jobData = jobData; }
  //! get job data
  const QByteArray &jobData() const { return m_jobData; }

  //! store track (for block jobs)
  void setTrack(int track) { m_track = track; }
  //! get track
  int track() const { return m_track; }
  //! store sector (for block jobs)
  void setSector(int sector) { m_sector = sector; }
  //! get sector
  int sector() const { return m_sector; }

  //! set track sector vector
  void setBlockPosVector(const BlockPosVector &tsv) { m_tsv = tsv; }
  //! set block count
  void setBlockCount(int blockCount) { m_blockCount = blockCount; }

  //! get current state of job
  int state() const { return m_state; }

protected:
  QByteArray m_packet;
  QByteArray m_block;
  Job m_job;
  int m_drive;
  QByteArray m_jobData;
  int m_track;
  int m_sector;
  int m_state;
  bool m_check;

  BlockPosVector m_tsv;
  int m_blockCount;

  // ----- High Level -----
  //! check if drive is ready
  Result checkDriveReady(NetHost &host,bool &driveReady);
  //! read drive status
  Result readStatus(NetHost &host,QByteArray &status);
  //! read directory
  Result readDirectory(NetHost &host,QByteArray &rawDir);
  //! send command to drive
  Result sendCommand(NetHost &host,const QByteArray &command);
  //! read disk block
  Result readDiskBlock(NetHost &host,int track,int sector,
                       QByteArray &block);
  //! write disk block
  Result writeDiskBlock(NetHost &host,int track,int sector,
                        const QByteArray &block);

  //! warp read blocks
  Result warpReadBlocks(NetHost &host,const BlockPosVector &tsv);
  //! warp read disk
  Result warpReadDisk(NetHost &host);
  //! warp write disk
  Result warpWriteDisk(NetHost &host,const QByteArray &rawImage);

  // ----- Low Level -----
  enum Command {
    DRIVE_READY_CMD     = 0x3f,
    OPEN_CMD            = 0x18,
    CLOSE_CMD           = 0x19,
    READ_CMD            = 0x1a,
    SKIP_READ_CMD       = 0x1b,  // skip 2 bytes and read 30 bytes or < if file end
    READ_BLOCK_CMD      = 0x31,
    WRITE_BLOCK_CMD     = 0x32,
    START_WARP_READ_CMD = 0x0c,
    STOP_WARP_READ_CMD  = 0x0d,
    WARP_READ_BLOCK_CMD = 0x0e,
    WARP_READ_DISK_CMD  = 0x08,
    WARP_WRITE_DISK_CMD = 0x28,
    WARP_STOP_CMD       = 0x01
  };
  //! fill packet with command and string argument
  void makeCommandPacketWithTrackSector(char command,char fileNo,char drive,char sec,
                                        int track,int sector);
  //! fill packet with command and string argument
  void makeCommandPacketWithString(char command,char fileNo,char drive,char sec,
                                   const QString &str="");
  //! file packet with command
  void makeCommandPacket(char command,char fileNo,char drive=0,char sec=0);
  //! send current packet
  Result sendPacket(NetHost &host);
  //! send current packet and wait for result
  Result sendPacketAndGetStatus(NetHost &host,char &status);
  //! send current packet and wait for result block
  Result sendPacketAndGetPacket(NetHost &host);
  //! send current packet and check status
  Result sendPacketAndCheckStatus(NetHost &host);
  //! receive a block
  Result recvBlock(NetHost &host);
  //! send a block
  Result sendBlock(NetHost &host);
  //! return data from a block
  QByteArray getBlockData(bool skipLastByte);
  //! check status
  Result checkStatus() const;
};

#endif
