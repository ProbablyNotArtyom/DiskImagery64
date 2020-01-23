#include "mainwin.h"
#include "dimagewin.h"
#include "filewin.h"
#include "version.h"

NetWin *MainWin::g_netWin = 0;

MainWin::MainWin(const QString &winClass,const QRect &defGeo,QWidget *parent) : QMainWindow(parent),
  m_winClass(winClass),
  m_defGeo(defGeo)
{
  setAttribute(Qt::WA_MacMetalStyle);
  setAttribute(Qt::WA_DeleteOnClose);

  initActions();
  initMenu();
  
  loadSettings();
  
  m_preferencesDialog = 0;
  
  // first main win create network window
  if(g_netWin==0) {
    g_netWin = new NetWin;
  }
  connect(qApp,SIGNAL(lastWindowClosed()),this,SLOT(lastWindowClosed()));
}

MainWin::~MainWin()
{
  saveSettings();
  delete m_preferencesDialog;
}

void MainWin::lastWindowClosed()
{
  // last main win deletes network window
  if(g_netWin!=0) {
    delete g_netWin;
    g_netWin = 0;
  }
}

void MainWin::initActions()
{
  // ----- File Menu -----
  m_newImageAction = new QAction(tr("&New Image"),this);
  m_newImageAction->setShortcut(tr("Ctrl+N"));
  connect(m_newImageAction,SIGNAL(triggered()),this,SLOT(newImage()));
  
  m_newBrowserAction = new QAction(tr("New &Browser"),this);
  m_newBrowserAction->setShortcut(tr("Ctrl+Shift+N"));
  connect(m_newBrowserAction,SIGNAL(triggered()),this,SLOT(newBrowser()));
  
  m_openImageAction = new QAction(tr("&Open Image"),this);
  m_openImageAction->setShortcut(tr("Ctrl+O"));
  connect(m_openImageAction,SIGNAL(triggered()),this,SLOT(openImage()));

  m_saveImageAction = new QAction(tr("&Save Image"),this);
  m_saveImageAction->setShortcut(tr("Ctrl+S"));
  
  m_saveImageAsAction = new QAction(tr("Save Image &as..."),this);
  
  m_closeAction = new QAction(tr("Close"),this);
  m_closeAction->setShortcut(tr("Ctrl+W"));
  connect(m_closeAction, SIGNAL(triggered()), this, SLOT(close()));
  
  m_quitAction = new QAction(tr("Quit"),this);
  m_quitAction->setShortcut(tr("Ctrl+Q"));
  connect(m_quitAction, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));
  
  // ----- Edit Menu -----
  m_cutAction = new QAction(tr("C&ut"),this);
  m_cutAction->setShortcut(tr("Ctrl+X"));
  
  m_copyAction = new QAction(tr("C&opy"),this);
  m_copyAction->setShortcut(tr("Ctrl+C"));
  
  m_pasteAction = new QAction(tr("&Paste"),this);
  m_pasteAction->setShortcut(tr("Ctrl+V"));

  m_deleteAction = new QAction(tr("&Delete"),this);
  QList<QKeySequence> delSeq;
  delSeq << QKeySequence("Backspace");
  delSeq << QKeySequence(QKeySequence::Delete);
  m_deleteAction->setShortcuts(delSeq);
  
  m_preferencesAction = new QAction(tr("&Preferences"),this);
  connect(m_preferencesAction, SIGNAL(triggered()), this, SLOT(preferences()));
  
  // ----- View Menu -----
  m_shiftCharsetAction = new QAction(tr("&Shift Charset"),this);
  m_shiftCharsetAction->setShortcut(tr("Ctrl+T"));
  m_shiftCharsetAction->setCheckable(true);
  
  m_showCharsetAction = new QAction(tr("Show &Charset"),this);
  m_showCharsetAction->setShortcut(tr("Alt+Ctrl+T"));
  
  // ----- Tools Menu -----
  m_formatDiskAction = new QAction(tr("&Format Disk"),this);
  m_formatDiskAction->setShortcut(tr("Ctrl+F"));
  
  m_addSeparatorAction = new QAction(tr("Add &Separator"),this);
  m_addSeparatorAction->setShortcut(tr("Ctrl+A"));
  
  // ----- Emulator Menu -----
  m_runProgramAction = new QAction(tr("&Run Program"),this);
  m_runProgramAction->setShortcut(tr("Ctrl+R"));

  m_mountImageAction = new QAction(tr("&Mount Image"),this);
  m_mountImageAction->setShortcut(tr("Ctrl+M"));
    
  // ----- Network Menu -----
  m_netRunProgramAction = new QAction(tr("&Run Program"),this);
  m_netRunProgramAction->setShortcut(tr("Ctrl+Shift+R"));
  connect(m_netRunProgramAction,SIGNAL(triggered()),this,SLOT(netRunProgram()));
  
  m_netShareFilesAction = new QAction(tr("&Share Files in Netdrive"),this);
  m_netShareFilesAction->setShortcut(tr("Ctrl+Shift+M"));
  connect(m_netShareFilesAction,SIGNAL(triggered()),this,SLOT(netShareFiles()));

  m_netShowLogAction = new QAction(tr("&Show Log Window"),this);
  m_netShowLogAction->setShortcut(tr("Ctrl+L"));
  connect(m_netShowLogAction,SIGNAL(triggered()),this,SLOT(netShowLog()));
  
  // WarpCopy
  m_netWCStartServerAction = new QAction(tr("Start WarpCopy"),this);
  connect(m_netWCStartServerAction,SIGNAL(triggered()),this,SLOT(netWCStartServer()));

  m_netWCWarpReadDiskAction = new QAction(tr("Read Disk (Warp)"),this);
  connect(m_netWCWarpReadDiskAction,SIGNAL(triggered()),this,SLOT(netWCWarpReadDisk()));

  m_netWCWarpWriteDiskAction = new QAction(tr("Write Disk (Warp)"),this);
  connect(m_netWCWarpWriteDiskAction,SIGNAL(triggered()),this,SLOT(netWCWarpWriteDisk()));
  
  m_netWCReadDiskAction = new QAction(tr("Read Disk (Slow)"),this);
  connect(m_netWCReadDiskAction,SIGNAL(triggered()),this,SLOT(netWCReadDisk()));

  m_netWCWriteDiskAction = new QAction(tr("Write Disk (Slow)"),this);
  connect(m_netWCWriteDiskAction,SIGNAL(triggered()),this,SLOT(netWCWriteDisk()));

  m_netWCFormatDiskAction = new QAction(tr("Format Disk"),this);
  connect(m_netWCFormatDiskAction,SIGNAL(triggered()),this,SLOT(netWCFormatDisk()));
  
  m_netWCVerifyDiskAction = new QAction(tr("Verify Disk"),this);
  connect(m_netWCVerifyDiskAction,SIGNAL(triggered()),this,SLOT(netWCVerifyDisk()));
  
  m_netWCSendDOSCommandAction = new QAction(tr("Send DOS Command"),this);
  connect(m_netWCSendDOSCommandAction,SIGNAL(triggered()),this,SLOT(netWCSendDOSCommand()));
  
  m_netWCGetDriveStatusAction = new QAction(tr("Get Drive Status"),this);
  connect(m_netWCGetDriveStatusAction,SIGNAL(triggered()),this,SLOT(netWCGetDriveStatus()));
  
  // ----- Help Menu ------
  m_aboutAction = new QAction(tr("&About"),this);
  connect(m_aboutAction,SIGNAL(triggered()),this,SLOT(about()));
}

void MainWin::initMenu()
{
  // ----- File Menu -----
  QMenu *menu = menuBar()->addMenu(tr("&File"));
  menu->addAction(m_newImageAction);
  menu->addAction(m_newBrowserAction);
  menu->addAction(m_openImageAction);
  menu->addSeparator();
  menu->addAction(m_saveImageAction);
  menu->addAction(m_saveImageAsAction);
  menu->addAction(m_closeAction);
  menu->addSeparator();
  menu->addAction(m_quitAction);
  // ----- Edit Menu -----
  menu = menuBar()->addMenu(tr("&Edit"));
  menu->addAction(m_cutAction);
  menu->addAction(m_copyAction);
  menu->addAction(m_pasteAction);
  menu->addAction(m_deleteAction);
  menu->addSeparator();
  menu->addAction(m_preferencesAction);
  // ----- View Menu -----
  menu = menuBar()->addMenu(tr("&View"));
  menu->addAction(m_shiftCharsetAction);
  menu->addAction(m_showCharsetAction);
  // ----- Tools Menu -----
  menu = menuBar()->addMenu(tr("&Tools"));
  menu->addAction(m_formatDiskAction);
  menu->addAction(m_addSeparatorAction);
  // ----- Emulator Menu -----
  menu = menuBar()->addMenu(tr("E&mulator"));
  menu->addAction(m_runProgramAction);
  menu->addAction(m_mountImageAction);
  // ----- Network Menu -----
  menu = menuBar()->addMenu(tr("&Network"));
  menu->addAction(m_netRunProgramAction);
  menu->addAction(m_netShareFilesAction);
  menu->addAction(m_netShowLogAction);
  QMenu *wcMenu = menu->addMenu(tr("WarpCopy64"));
  wcMenu->addAction(m_netWCStartServerAction);
  wcMenu->addSeparator();
  wcMenu->addAction(m_netWCWarpReadDiskAction);
  wcMenu->addAction(m_netWCWarpWriteDiskAction);
  wcMenu->addAction(m_netWCReadDiskAction);
  wcMenu->addAction(m_netWCWriteDiskAction);
  wcMenu->addSeparator();
  wcMenu->addAction(m_netWCFormatDiskAction);
  wcMenu->addAction(m_netWCVerifyDiskAction);
  wcMenu->addAction(m_netWCSendDOSCommandAction);
  wcMenu->addAction(m_netWCGetDriveStatusAction);
  // ----- Help Menu -----
  menu = menuBar()->addMenu(tr("&Help"));
  menu->addAction(m_aboutAction);
}

// ----- Slots -----

void MainWin::newImage()
{
  DImageWin *win = new DImageWin(DImage::D64);
  win->show();
}

void MainWin::newBrowser()
{
  FileWin *win = new FileWin;
  win->show();
}

void MainWin::openImage()
{
  QString fileName = QFileDialog::getOpenFileName(this,tr("Open Image File"));
  if(fileName!="") {
    DImage::DiskFormat format = DImage::determineDiskFormat(fileName);
    if(format != DImage::INVALID) {
      DImageWin *win = new DImageWin(fileName);
      win->show();
    } else {
      QMessageBox::warning(this,tr("Unknown Image Format"),
                           tr("Format of File '%1' has no supported Image Type!")
                           .arg(fileName));
    }
  }
}

void MainWin::about()
{
  QMessageBox *box = new QMessageBox(this);
  box->setStandardButtons(QMessageBox::Ok);
  box->setIconPixmap(QPixmap(":/imagery/imagery.png"));
  box->setWindowTitle(tr("About DiskImagery64"));
  box->setText(tr("<b>Disk Imagery 64 - Version " VERSION "</b><br>"
                  "<small>Written by Christian Vogelgsang<br>"
                  "Contact: <tt>chris@vogelgsang.org</tt><br>"
                  "Homepage: <a href=\"http://www.lallafa.de/blog\">http://www.lallafa.de/blog</a><br>"
                  "Licensed under the GNU Public License V2<br><br>"
                  "Uses: diskimage D64/D71/D81 library<br>"
                  "Copyright (c) 2003-2006, Per Olofsson<br>"
                  "All rights reserved.<br><br>"
                  "Special Thanks to<br>"
                  "John \"Graham\" Selck and Stefan \"j0x\" Wolff<br>"
                  "for sharing their networking source code!<br><br>"
                  "Greetings to all Mac+CBM Addicts out there!"
                  "</small>"
                  ));
  box->exec();
  delete box;
}

void MainWin::preferences()
{
  if(m_preferencesDialog==0) {
    m_preferencesDialog = new Preferences(this);
  }
  m_preferencesDialog->load();
  if(m_preferencesDialog->exec()==QDialog::Accepted) {
    m_preferencesDialog->save();
  }
}

void MainWin::operateOnFile(const CBMFile &file)
{
  // double click triggers a net run
  netRunFile(file);
}

// network

void MainWin::netShowLog()
{
  g_netWin->show();
}

void MainWin::netRunProgram()
{
  CBMFile file;
  if(!getCurrentFile(file)) {
    QMessageBox::warning(this,tr("Net Run Program"),
                         tr("Please select a file to run!"));
    return;
  }
  return netRunFile(file);
}

void MainWin::netRunFile(const CBMFile &file)
{
  quint16 addr;
  QByteArray data;
  if(!file.prgRunAddress(addr) || !file.prgData(data)) {
    QMessageBox::warning(this,tr("Net Run Program"),
                         tr("Cannot query PRG file!"));
    return;
  }
  if(data.size()==0) {
    QMessageBox::warning(this,tr("Net Run Program"),
                         tr("Can not run empty PRG file!"));
    return;
  }

  netShowLog();
  g_netWin->runProgram(addr,data);
}

void MainWin::netShareFiles()
{
  CBMFileList files;
  if(!getCurrentFiles(files)) {
    QMessageBox::warning(this,tr("Net Share Files"),
                         tr("Please select some files first!"));
    return;
  }
  
  netShowLog();
  g_netWin->shareFiles(files);
}

// ----- warp copy -----

void MainWin::netWCStartServer()
{
  QString wcPrg;
  bool wcPatch;
  Preferences::getWarpCopyDefaults(wcPrg,wcPatch);
  
  // read prg
  CBMFile file;
  if(!file.fromLocalFile(wcPrg)) {
    QMessageBox::warning(this,tr("Start WarpCopy"),
                         tr("Can't access WarpCopy PRG:\n%1")
                         .arg(wcPrg));
    return;
  }
  
  // patch addresses
  if(wcPatch) {
    QByteArray data = file.data();
    if(!patchWarpCopy(data)) {
      QMessageBox::warning(this,tr("Start WarpCopy"),
                           tr("Unable to patch WarpCopy PRG!"));
      return;
    }
    file.setData(data);
  }
  
  // run file via codenet
  return netRunFile(file);
}

bool MainWin::patchWarpCopy(QByteArray &data)
{
  // warp copy original address:
  const char oldC64[4] = { (char)192, (char)168, 0, 64 };
  
  // new addresses:
  AddrPair addrPair;
  Preferences::getNetworkDefaults(addrPair);
  char newC64[4];
  quint32 addrC64 = addrPair.c64Addr.toIPv4Address();
  newC64[0] = (char)((addrC64>>24)&0xff);
  newC64[1] = (char)((addrC64>>16)&0xff);
  newC64[2] = (char)((addrC64>>8)&0xff);
  newC64[3] = (char)( addrC64 &0xff);
  
  if(data.size()<4)
    return false;
  
  int foundC64 = 0;
  int i,j;
  for(i=0;i<data.size()-4;i++) {
    int gotC64=0;
    for(j=0;j<4;j++) {
      if(data[i+j]==oldC64[j])
        gotC64++;
    }
    if(gotC64==4) {
      foundC64++;
      for(j=0;j<4;j++) {
        data[i+j] = newC64[j];
      }
    }
  }
  return (foundC64==1);
}

void MainWin::netWCWarpReadDisk()
{
  // create temp image
  DImageWin *newImgWin = new DImageWin(DImage::D64);  
  netShowLog();
  if(g_netWin->warpReadDisk(newImgWin->getEmbeddedDImage())) {
    newImgWin->updateDImage();
    newImgWin->show();
  } else {
    delete newImgWin;
  }
}

void MainWin::netWCWarpWriteDisk()
{
  DImage *image = getEmbeddedDImage();
  if(image==0) {
    QMessageBox::warning(this,tr("Warp Write Disk"),
                         tr("No Image to write found!"));
    return;
  }
  
  netShowLog();
  g_netWin->warpWriteDisk(image);
}

void MainWin::netWCReadDisk()
{
  // create temp image
  DImageWin *newImgWin = new DImageWin(DImage::D64);  
  netShowLog();
  if(g_netWin->readDisk(newImgWin->getEmbeddedDImage())) {
    newImgWin->updateDImage();
    newImgWin->show();
  } else {
    delete newImgWin;
  }
}

void MainWin::netWCWriteDisk()
{
  DImage *image = getEmbeddedDImage();
  if(image==0) {
    QMessageBox::warning(this,tr("Warp Write Disk"),
                         tr("No Image to write found!"));
    return;
  }
  
  netShowLog();
  g_netWin->writeDisk(image);
}

// --- other wc ops ---

void MainWin::netWCFormatDisk()
{
  QString nameId = QInputDialog::getText(0,tr("Format Disk"),
                                         tr("Name,(Id):"));
  if(nameId=="")
    return;
  QStringList items = nameId.split(",");
  int num = items.size();
  if(num==0)
    return;
  QString name,id;
  name = items.at(0);
  if(num>1)
    id = items.at(1);

  netShowLog();
  g_netWin->formatDisk(name,id);
}

void MainWin::netWCVerifyDisk()
{
  netShowLog();
  g_netWin->verifyDisk();
}

void MainWin::netWCSendDOSCommand()
{
  QString cmd = QInputDialog::getText(0,tr("Send DOS Command"),
                                      tr("CBM DOS Command:"));
  if(cmd=="")
    return;
  
  netShowLog();
  g_netWin->sendDOSCommand(cmd);
}

void MainWin::netWCGetDriveStatus()
{
  netShowLog();
  g_netWin->getDriveStatus();
}

// ----- Settings -----

void MainWin::saveSettings()
{
  QSettings settings;
  settings.beginGroup(m_winClass);
  
  // geometry
  settings.setValue("geometry",geometry());
  
  settings.endGroup();
}

void MainWin::loadSettings()
{
  QSettings settings;
  settings.beginGroup(m_winClass);

  // geometry
  QRect rect = settings.value("geometry",m_defGeo).toRect();
  move(rect.topLeft());
  resize(rect.size());
  
  settings.endGroup();
}
