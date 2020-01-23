#ifndef CODENET_H
#define CODENET_H

#include <QtNetwork/QtNetwork>
#include "netservice.h"

#include <QObject>

class CodeNetService : public NetService
{
  Q_OBJECT

public:
  CodeNetService();
  ~CodeNetService();

  // code net services:
  void sendData(quint16 addr,const QByteArray &data);
  void fillRegion(quint16 addr,quint16 size,quint8 value);
  void execJump(quint16 addr);
  void execRun();

protected:
  quint8 m_seqNum;
  QByteArray m_packet;

  void sendCommand(quint8 cmd);
  void sendBlock(quint16 addr,const QByteArray &data);
  
  quint8 lo(quint16 x)
  { return (quint8)(x & 0xff); }
  quint8 hi(quint16 x)
  { return (quint8)(x >> 8); }
  quint16 hilo(quint8 h,quint8 l)
  { return (quint16)h << 8 | (quint16)l; }
};

class CodeNetTask : public NetTask
{
public:
  CodeNetTask(const QByteArray &packet);
  ~CodeNetTask();
  
  virtual Result run(NetHost &host);

protected:
  QByteArray m_packet;
};

#endif
