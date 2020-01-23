#include "netdrivedevice.h"
#include "rawdir.h"

NetDriveDevice::NetDriveDevice(const CBMFileList &files)
: m_files(files)
{
  initStreams();
  setMessage(OK);
  m_dirBuffer.resize(4096);
}

NetDriveDevice::~NetDriveDevice()
{
  freeStreams();
}

// interface for net drive

NetDriveDevice::Result NetDriveDevice::open(const QString &name,quint8 channel)
{
  OpenMode openMode;
  CBMFile::Type fileType;
  
  // already open?
  if(isStreamOpen(channel)) {
    setMessage(NO_CHANNEL_AVAILABLE);
    return ND_ERROR_FILE_OPEN;
  }
  
  // get open mode and file type
  openMode = READ;
  if(channel==1)
    openMode = WRITE;
  fileType = CBMFile::PRG;
  QString fileName = parseTypeMode(name,fileType,openMode);
  
  // command channel?
  if(channel==15) {
    if(name!="") {
      // handle command
      if(!handleCommand(name)) {
        setMessage(SYNTAX_ERROR);
        return ND_DONE;
      }
    }
    // prepare status message
    QByteArray msg;
    createMessage(msg);
    openStream(15,-1,msg,READ);
  }
  // normal channel
  else {
    // directory?
    if((fileName=="$")&&(openMode==READ)) {
      RawDir rawDir;
      rawDir.fromFileList(m_files);
      openStream(channel,-1,rawDir.rawData(),READ);
    } else {
      // search file!
      int pos = searchFile(fileName,fileType);
      if(pos==-2) {
        setMessage(FILE_TYPE_MISMATCH);
        return ND_ERROR_FILE_NOT_FOUND;
      }
      if(openMode==READ) {
        // read file not found!
        if(pos==-1) {
          setMessage(FILE_NOT_FOUND);
          return ND_ERROR_FILE_NOT_FOUND;
        } else {
          // read file
          openStream(channel,pos,m_files.at(pos).data(),READ);
        }
      } else if(openMode==WRITE) {
        // file exists?
        if(pos!=-1) {
          setMessage(FILE_EXISTS);
          return ND_ERROR_FILE_OPEN;
        } else {
          // write file
          CBMFile file(fileName,fileType);
          m_files << file;
          openStream(channel,m_files.size()-1,file.data(),WRITE);
        }
      } else if(openMode==APPEND) {
        // file not found?
        if(pos==-1) {
          CBMFile file(fileName,fileType);
          m_files << file;
          openStream(channel,m_files.size()-1,file.data(),APPEND);
        } else {
          openStream(channel,pos,m_files.at(pos).data(),APPEND);
        }
      }
    }
  }
  
  setMessage(OK);
  return ND_DONE;
}

NetDriveDevice::Result NetDriveDevice::close(quint8 channel)
{
  if(!isStreamOpen(channel)) {
    setMessage(FILE_NOT_OPEN);
    return ND_ERROR_FILE_NOT_OPEN;
  }
  
  closeStream(channel);
  setMessage(OK);
  return ND_DONE;
}

NetDriveDevice::Result NetDriveDevice::offset(quint8 channel,quint8 pos)
{
  if(!isStreamOpen(channel)) {
    setMessage(FILE_NOT_OPEN);
    return ND_ERROR_FILE_NOT_OPEN;
  }
  
  m_streams[channel]->offset(pos);
  setMessage(OK);
  return ND_DONE;
}

NetDriveDevice::Result NetDriveDevice::read(quint8 channel,quint8 size,
                                            QByteArray &data)
{
  if(!isStreamOpen(channel)) {
    setMessage(FILE_NOT_OPEN);
    return ND_ERROR_FILE_NOT_OPEN;
  }
  
  OpenMode openMode = m_streams[channel]->openMode();
  if(openMode!=READ) {
    setMessage(READ_ERROR);
    return ND_ERROR_FILE_OPEN;
  }
  
  m_streams[channel]->read(size,data);
  setMessage(OK);
  return ND_DONE;
}

NetDriveDevice::Result NetDriveDevice::write(quint8 channel,
                                             const QByteArray &data)
{
  if(!isStreamOpen(channel)) {
    setMessage(FILE_NOT_OPEN);
    return ND_ERROR_FILE_NOT_OPEN;
  }
  
  OpenMode openMode = m_streams[channel]->openMode();
  if((openMode!=WRITE)||(openMode!=APPEND)) {
    setMessage(WRITE_ERROR);
    return ND_ERROR_FILE_OPEN;
  }
  
  m_streams[channel]->write(data);
  setMessage(OK);
  return ND_DONE;
}

// ----- Tools -----

QString NetDriveDevice::parseTypeMode(const QString &name,
                                      CBMFile::Type &type,
                                      OpenMode &mode)
{
  QString result;
  int commas = name.count(',');
  int size = name.size();
  int pos = 0;
  if(commas>0) {
    pos = name.indexOf(',');
    result = name.left(pos);
    if(pos<size-1) {
      switch(name[pos+1].unicode()) {
      default:
        type = CBMFile::UNKNOWN;
        break;
      case 'P':
        type = CBMFile::PRG;
        break;
      case 'S':
        type = CBMFile::SEQ;
        break;
      case 'U':
        type = CBMFile::USR;
        break;
      }
    }
  }
  else if(commas>1) {
    pos = name.indexOf(',',pos+1);
    if(pos<size-1) {
      switch(name[pos+1].unicode()) {
      default:
      case 'R':
        mode = READ;
        break;
      case 'W':
        mode = WRITE;
        break;
      case 'A':
        mode = APPEND;
        break;
      }
    }
  }
  else
    result = name;
  return result;
}

bool NetDriveDevice::handleCommand(const QString &cmd)
{
  qDebug("command: '%s'",cmd.toLocal8Bit().data());
  return true;
}

void NetDriveDevice::createMessage(QByteArray &data)
{
  int size = m_message.size();
  data.resize(size);
  for(int i=0;i<size;i++)
    data[i] = m_message[i].unicode();
}

int NetDriveDevice::searchFile(const QString &glob,CBMFile::Type type)
{
  qDebug("search file glob '%s'",glob.toLocal8Bit().data());
  QRegExp globPat(glob,Qt::CaseSensitive,QRegExp::Wildcard);
  for(int i=0;i<m_files.size();i++) {
    const CBMFile &file = m_files.at(i);
    if(file.type()==type) {
      if(globPat.exactMatch(file.name())) {
        qDebug(" matched: '%s'",file.name().toLocal8Bit().data());
        return i;
      }
    } else {
      if(file.name()==glob)
        return -2; // file type mismatch
    }
  }
  return -1;
}

// ----- Directory Tools -----


// ----- Stream Tools -----

void NetDriveDevice::initStreams()
{
  for(int i=0;i<16;i++) {
    m_streams[i] = 0;
  }  
}

void NetDriveDevice::freeStreams()
{
  for(int i=0;i<16;i++) {
    if(m_streams[i]!=0) {
      qDebug("force close stream @%d",i);
      delete m_streams[i];
      m_streams[i] = 0;
    }
  }  
}

bool NetDriveDevice::isStreamOpen(quint8 channel) const
{
  return (m_streams[channel]!=0);
}

void NetDriveDevice::openStream(quint8 channel,
                                int fileId,
                                const QByteArray &data,
                                OpenMode openMode)
{
  m_streams[channel] = new NetDriveStream(fileId,data,openMode);
}

void NetDriveDevice::closeStream(quint8 channel)
{
  delete m_streams[channel];
  m_streams[channel] = 0;
}

// ----- Messages -----

void NetDriveDevice::setMessage(Message msg)
{
  const char *txt;
  switch(msg) {
  case OK:                    txt = "OK"; break;
  case SYNTAX_ERROR:          txt = "SYNTAX ERROR"; break;
  case READ_ERROR:            txt = "READ ERROR"; break;
  case WRITE_ERROR:           txt = "WRITE ERROR"; break;
  case FILE_NOT_OPEN:         txt = "FILE NOT OPEN"; break;
  case FILE_NOT_FOUND:        txt = "FILE NOT FOUND"; break;
  case FILE_EXISTS:           txt = "FILE EXISTS"; break;
  case FILE_TYPE_MISMATCH:    txt = "FILE TYPE MISMATCH"; break;
  case NO_CHANNEL_AVAILABLE:  txt = "NO CHANNEL AVAILABLE"; break;
  }
  m_message.sprintf("%02d,%s,00,00",msg,txt);
}

// ========== NetDriveStream ================================================

NetDriveStream::NetDriveStream(int fileId,
                               const QByteArray &data,
                               NetDriveDevice::OpenMode openMode)
: m_fileId(fileId),
  m_data(data),
  m_openMode(openMode),
  m_pos(0),
  m_lastPos(0)
{
}

NetDriveStream::~NetDriveStream()
{
}

void NetDriveStream::offset(quint8 pos)
{
  m_pos = m_lastPos + pos;
}

void NetDriveStream::read(quint8 size,QByteArray &data)
{
  quint32 startPos = m_pos;
  quint32 bufferSize = m_data.size();
  // end of file
  if(startPos>=bufferSize)
    return;

  quint32 endPos = startPos + size;
  if(endPos >= bufferSize)
    endPos = bufferSize;
  
  quint32 realSize = endPos - startPos;
  data.resize(realSize);
  for(quint32 i=0;i<realSize;i++)
    data[i] = m_data[startPos+i];
  
  m_lastPos = m_pos;
  m_pos += realSize;
}

void NetDriveStream::write(const QByteArray &data)
{
  quint32 newSize = data.size();
  quint32 oldSize = m_data.size();
  m_data.resize(oldSize + newSize);
  for(quint32 i=0;i<newSize;i++)
    m_data[oldSize+i] = data[i];
}

