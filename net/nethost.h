#ifndef NETHOST_H
#define NETHOST_H

#include <QtNetwork/QtNetwork>

#include <QObject>

struct AddrPair {
  QHostAddress myAddr;
  quint16 myPort;
  QHostAddress c64Addr;
  quint16 c64Port;
  
  bool operator == (const AddrPair &ap) const
  { return (myAddr==ap.myAddr)  && (myPort==ap.myPort) &&
           (c64Addr==ap.c64Addr) && (c64Port==ap.c64Port); }
  bool operator != (const AddrPair &ap) const
  { return (myAddr!=ap.myAddr)  || (myPort!=ap.myPort) &&
           (c64Addr!=ap.c64Addr) || (c64Port!=ap.c64Port); }
};

class NetHost : public QObject
{
  Q_OBJECT

public:
  NetHost();
  ~NetHost();
  
  void setTimeout(int msec) { m_timeout = msec; }
  int timeout() const { return m_timeout; }
  
  bool bind(const AddrPair &addrPair);
  void close();
  bool isBound() const;

  bool sendPacket(const QByteArray &packet);
  bool receivePacket(QByteArray &packet);
  bool waitForPacket(int timeout);
  bool hasPackets();
  int  flushPackets(int timeout);

  QString errorString() const { return m_errorString; }

signals:
  void sentPacket(int size);
  void receivedPacket(int size);
  
protected:
  QUdpSocket m_socket;
  AddrPair   m_addrPair;
  int        m_timeout;
  QString    m_errorString;
};

#endif
