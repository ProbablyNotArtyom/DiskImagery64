#ifndef DImageWin_H
#define DImageWin_H

#include <QtGui/QtGui>
#include "dimagemodel.h"
#include "mainwin.h"

class DImageWin : public MainWin
{
  Q_OBJECT
  
public:
  DImageWin(DImage::DiskFormat format,QWidget *parent=0);
  DImageWin(const QString &fileName,QWidget *parent=0);
  ~DImageWin();

  //! return my dimage
  virtual DImage *getEmbeddedDImage() { return &m_dimage; }
  
public slots:
  void updateDImage();
  
  bool saveImage();
  bool saveImageAs();

  void cut();
  void copy();
  void paste();
  void deleteSel();

  void shiftCharset(bool on);
  void showCharset();

  void formatDisk();
  void addSeparator();
  void mountImage();
  void runProgram();

protected slots:
  //! double click on item
  void activateItem(const QModelIndex &);

protected:
  //! embedded disk image object
  DImage m_dimage;
  
  //! window icon file
  QIcon m_fileIcon;
  //! darkened window icon
  QIcon m_darkFileIcon;
  //! remember the original font
  QString m_origFont;
  //! font flag: is shifted
  bool m_fontShifted;

  //! ui: disk title label
  QLabel *m_diskTitle;
  //! ui: model for disk directory
  DImageModel *m_model;
  //! ui: tree view for disk model
  QTreeView *m_dirView;
  //! ui: blocks free label
  QLabel *m_blocksFree;
  //! ui: status message
  QLabel *m_driveStatus;
  
  // charset dialog
  QDialog  *m_charsetDialog;
  QLabel   *m_charsetChars;
  
  // format image dialog
  QDialog   *m_formatImageDialog;
  QLineEdit *m_formatImageName;
  QLineEdit *m_formatImageId;
  
  // add separator dialog
  QDialog   *m_addSeparatorDialog;
  QComboBox *m_addSeparatorCombo;
  QProcess   m_emuProcess;
  
  QPixmap darkenPixmap(const QPixmap &pixmap);
  void init();
  void closeEvent(QCloseEvent *event);
  void updateFont();
  void runEmu(const QString &app,const QString &args,const QString &fileName="");
  QString replaceArgTags(const QString &str,const QString &fileName="");
  QFont currentFont();
  
  //! return current cbm file
  virtual bool getCurrentFile(CBMFile &file);
  //! return current cbm files
  virtual bool getCurrentFiles(CBMFileList &files);
};

#endif
