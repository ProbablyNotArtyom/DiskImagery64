#ifndef NETDRIVE_H
#define NETDRIVE_H

#include <QtNetwork/QtNetwork>
#include "netservice.h"
#include "netdrivedevice.h"

#include <QObject>

class NetDriveService : public NetService
{
  Q_OBJECT

public:
  NetDriveService();
  ~NetDriveService();

  // netdrive services:
  void shareFiles(const CBMFileList &files);

protected:
  NetDriveDevice *m_device;
};

class NetDriveTask : public NetTask
{
public:
  NetDriveTask(const CBMFileList &files);
  ~NetDriveTask();
  
  virtual Result run(NetHost &host);

protected:
  NetDriveDevice *m_device;
  QByteArray m_packet;
  quint8 m_seqNum;
  enum Command { OPEN=1,CHKIN=2,READ=3,CLOSE=4,WRITE=5,DATA=0x20 };
  
  bool checkSignature(const QByteArray &packet);
  QString getFileName(const QByteArray &packet,quint8 size);
};

#endif
