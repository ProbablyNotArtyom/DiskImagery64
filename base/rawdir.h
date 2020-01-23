#ifndef RAWDIR_H
#define RAWDIR_H

#include "cbmfile.h"

class RawDir {
public:
  RawDir();
  ~RawDir();
  
  //! set skip load address
  void setSkipLoadAddr(bool skip) { m_skipLoadAddr = skip; }
  //! do skip load addr
  bool skipLoadAddr() const { return m_skipLoadAddr; }
  
  //! convert raw dir to file list
  bool toFileList(CBMFileList &fileList);
  //! convert a file list to a raw dir
  void fromFileList(const CBMFileList &fileList);

  //! access raw dir
  const QByteArray &rawData() const { return m_dirBuffer; }
  //! set raw dir
  void setRawData(const QByteArray &rawDir) { m_dirBuffer = rawDir; }

protected:
  quint32 m_dirBufPos;
  QByteArray m_dirBuffer;
  bool m_skipLoadAddr;
  
  void writeByte(quint8 b);
  void writeWord(quint16 w);
  void writeString(const QString &string,int pad);
  void writeStringPad(const QString &string,int pad);
  
  bool readByte(quint8 &b);
  bool readWord(quint16 &w);
  bool readString(int size,QString &str);
  bool readBytes(int size,QByteArray &data);
  bool checkByte(quint8 b);
  bool checkWord(quint16 w);
  
  bool readHeader(QString &name,QString &id);
  bool readFile(CBMFile &file,quint16 blocks);
  int calcNumDigits(quint16 num);
};

#endif
