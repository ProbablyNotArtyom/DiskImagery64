// dimage.h
// disk image wrapper class

#ifndef DIMAGE_H
#define DIMAGE_H

#include <QtCore/QtCore>
#include "cbmfile.h"

extern "C" {
#include "diskimage.h"
}

//! a block map is a vector with size=number of tracks.
//! each entry contains the sector number
typedef QVector<int> BlockMap;
typedef QVector<int> OffsetMap;

class DImage {
public:
  //! format
  enum DiskFormat { D64, D71, D81, INVALID };
  
  //! create a new image
  DImage(DiskFormat f);
  //! open image if file exists or create a new one
  DImage(const QString &fileName);
  //! remove disk image and sync if file existed
  ~DImage();
  
  //! write image to disk
  void sync();
  //! format the image
  void format(const QString &name,const QString &id="");
  //! return status
  QString status() const;
  
  //! is valid
  bool isValid() const { return m_diskImage != 0; }
  //! is disk image already presented as a file
  bool isFileAvailable() const { return m_fileAvailable; }
  //! is image dirty?
  bool isDirty() const { return m_dirty; }
  
  //! return format
  DiskFormat diskFormat() const { return m_format; }
  //! file name
  QString fileName() const { return m_fileName; }
  //! set file name
  void setFileName(const QString &fileName);
  //! set dirty state
  void setDirty(bool dirty) { m_dirty=dirty; }
  
  //! blocks free
  int blocksFree() const;
  //! disk name
  void diskTitle(QString &name,QString &id) const;

  //! read directory
  bool readDirectory(CBMFileList &files);
  
  //! read a file
  bool readFile(CBMFile &file);
  //! write a file
  bool writeFile(const CBMFile &file);
  //! delete a file
  bool deleteFile(const CBMFile &file);
  //! renamve a file
  bool renameFile(CBMFile &file,const QString &newName);

  //! determine format from file
  static DiskFormat determineDiskFormat(const QString &fileName);
  //! get file extension for format
  static QString fileExtensionForDiskFormat(DiskFormat format);
  //! get file filter for format
  static QString fileFilterForDiskFormat(DiskFormat format);
  
  /** @name Raw Access */
  //@{
  //! return number of tracks
  int numTracks() const;
  //! return number of sectors of given track
  int numSectors(int track) const;
  //! return total number of blocks in image
  int numBlocks() const;
  //! return the block map
  BlockMap blockMap() const;
  //! return the offset map
  OffsetMap offsetMap(int *numBlocks=0) const;
  //! return block number for track sector
  int blockNum(int track,int sector) const;
  //! write a block
  void writeBlock(int block,const QByteArray &data);
  //! read a block
  void readBlock(int block,QByteArray &data) const;
  //! read to raw image
  void putToRawImage(QByteArray &image);
  //! write from raw image
  bool getFromRawImage(const QByteArray &image);
  //@}
  
protected:
  //! file name of image
  QString m_fileName;
  //! format
  DiskFormat m_format;
  //! internal disk image
  DiskImage *m_diskImage;
  //! has file on disk
  bool m_fileAvailable;
  //! is image dirty
  bool m_dirty;
  
  //! create a new image
  void createNewImage();  

  //! convert file name to raw name
  void convertFileNameToRaw(const QString &str,unsigned char *raw) const;
  //! convert raw name to file name
  void convertRawToFileName(unsigned char *raw,QString &str) const;
  //! convert id to raw id
  void convertIdToRaw(const QString &id,unsigned char *raw) const;
  //! convert raw id to id
  void convertRawToId(unsigned char *raw,QString &id) const;
  
  //! convert file type constant
  filetype getFileType(CBMFile::Type t);
};

#endif
