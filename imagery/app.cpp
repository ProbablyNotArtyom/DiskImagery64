#include "app.h"
#include "dimagewin.h"
#include "filewin.h"

App::App(int &argc,char **argv) : QApplication(argc,argv)
{
  setOrganizationName("Lallafa");
  setOrganizationDomain("lallafa.org");
  setApplicationName("DiskImagery64");

  if(argc>1) {
    for(int i=1;i<argc;i++) {
      QString fileName = QString::fromLocal8Bit(argv[i]);
      openFile(fileName);
    }
  } else {
    FileWin *win = new FileWin;
    win->show();
  }
}

App::~App()
{
}

bool App::event(QEvent *event)
{
  if(event->type() == QEvent::FileOpen) {
    handleFileOpenEvent(static_cast<QFileOpenEvent *>(event));
    return true;
  } else {
    return QApplication::event(event);
  }
}

void App::handleFileOpenEvent(QFileOpenEvent *event)
{
  QString fileName = event->file();
  openFile(fileName);
}

void App::openFile(const QString &fileName)
{
  QFileInfo info(fileName);
  if(info.isDir()) {
    FileWin *win = new FileWin(fileName);
    win->show();
  }
  else if(info.isFile() && info.isReadable()) {
    if(DImage::determineDiskFormat(fileName)!=DImage::INVALID) {
      DImageWin *win = new DImageWin(fileName);
      win->show();
    }
  }
  else {
    QMessageBox::warning(0,tr("Unknown File"),
                         tr("Can't open unknown File '%1'!").arg(fileName));
  }
}

int App::getNumFileWin()
{
  int num = 0;
  foreach(QWidget *w,QApplication::topLevelWidgets()) {
    if(qobject_cast<FileWin *>(w))
      num++;
  }
  return num;
}

int App::getNumDImageWin()
{
  int num = 0;
  foreach(QWidget *w,QApplication::topLevelWidgets()) {
    if(qobject_cast<DImageWin *>(w))
      num++;
  }
  return num;
}
