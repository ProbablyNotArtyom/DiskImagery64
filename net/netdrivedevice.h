#ifndef NETDRIVEDEVICE_H
#define NETDRIVEDEVICE_H

#include "cbmfile.h"

class NetDriveStream;

class NetDriveDevice
{
public:
  enum OpenMode { READ=0,WRITE=1,APPEND=2 };
  enum Result { 
    ND_DONE=0,
    ND_ERROR_NET=1,
    ND_ERROR_FILE_OPEN=2,
    ND_ERROR_FILE_NOT_OPEN=3,
    ND_ERROR_FILE_NOT_FOUND=4
  };

  NetDriveDevice(const CBMFileList &files);
  ~NetDriveDevice();
  
  // return (altered) file list on shutdown
  CBMFileList fileList() const { return m_files; }
  
  // interface for net drive
  Result open(const QString &name,quint8 channel);
  Result close(quint8 channel);
  Result offset(quint8 channel,quint8 pos);
  Result read(quint8 channel,quint8 size,QByteArray &data);
  Result write(quint8 channel,const QByteArray &data);
  
protected:
  CBMFileList m_files;
  NetDriveStream *m_streams[16];

  QByteArray m_dirBuffer;
  quint32 m_dirBufPos;

  QString m_message;
  enum Message {
    OK = 0,
    SYNTAX_ERROR = 30,
    READ_ERROR = 27,
    WRITE_ERROR = 28,
    FILE_NOT_OPEN = 61,
    FILE_NOT_FOUND = 62,
    FILE_EXISTS = 63,
    FILE_TYPE_MISMATCH = 64,
    NO_CHANNEL_AVAILABLE = 70,    
  };
  
  void setMessage(Message msg);
  void createMessage(QByteArray &data);

  QString parseTypeMode(const QString &name,CBMFile::Type &type,OpenMode &mode);
  
  void initStreams();
  void freeStreams();
  bool isStreamOpen(quint8 channel) const;
  void openStream(quint8 channel,
                  int fileId,
                  const QByteArray &data,
                  OpenMode openMode);
  void closeStream(quint8 channel);
  
  bool handleCommand(const QString &cmd);
  int searchFile(const QString &glob,CBMFile::Type type);
};

class NetDriveStream
{
public:
  NetDriveStream(int fileId,
                 const QByteArray &data,
                 NetDriveDevice::OpenMode openMode);
  ~NetDriveStream();
  
  int fileId() const { return m_fileId; }
  QByteArray data() const { return m_data; }
  NetDriveDevice::OpenMode openMode() const { return m_openMode; }
  
  void offset(quint8 pos);
  void read(quint8 size,QByteArray &data);
  void write(const QByteArray &data);
  
protected:
  int m_fileId;
  QByteArray m_data;
  NetDriveDevice::OpenMode m_openMode;
  quint32 m_pos;
  quint32 m_lastPos;
};

#endif
