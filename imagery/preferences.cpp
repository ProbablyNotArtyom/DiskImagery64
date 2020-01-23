#include "preferences.h"

#include <QWidget>
#include <QDialog>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QTreeView>
#include <QAbstractItemView>
#include <QHeaderView>
#include <QAction>
#include <QPixmap>
#include <QDialogButtonBox>
#include <QComboBox>
#include <QWidget>
#include <QLineEdit>
#include <QFileDialog>
#include <QMenu>
#include <QMenuBar>
#include <QInputDialog>
#include <QToolButton>
#include <QFontDialog>
#include <QListView>
#include <QGroupBox>
#include <QLineEdit>
#include <QCheckBox>

Preferences::Preferences(QWidget *parent)
: QDialog(parent)
{
  setWindowTitle("DiskImagery64: " + tr("Preferences"));
  setWindowIcon(QIcon(":/imagery/imagery-16.png"));

  // dialog groups
  QVBoxLayout *dialogLayout = new QVBoxLayout(this);
  QTabWidget *tabWidget = new QTabWidget(this);
  dialogLayout->addWidget(tabWidget,1);

  QWidget *group = createImageGroup(this);
  tabWidget->addTab(group,tr("Image"));

  group = createFontGroup(this);
  tabWidget->addTab(group,tr("Font"));

  group = createSeparatorGroup(this);
  tabWidget->addTab(group,tr("Separator"));

  group = createEmulatorGroup(this);
  tabWidget->addTab(group,tr("Emulator"));

  group = createNetworkGroup(this);
  tabWidget->addTab(group,tr("Network"));

  // dialog buttons
  QDialogButtonBox *box = new QDialogButtonBox(QDialogButtonBox::Ok|
                                               QDialogButtonBox::Cancel,
                                               Qt::Horizontal,
                                               this);
  connect(box,SIGNAL(accepted()),this,SLOT(accept()));
  connect(box,SIGNAL(rejected()),this,SLOT(reject()));
  dialogLayout->addWidget(box,0);
}

Preferences::~Preferences()
{
}

// ----- create UI -----

QWidget *Preferences::createImageGroup(QWidget *parent)
{  
  QString imgInfoMsg = tr("<small>Use <b>%c</b> or <b>%C</b> for Image counter (%3c for padding)</small>");
  QGroupBox *imgGroup = new QGroupBox(tr("New Image Naming Scheme"),parent);
  QLabel *imgFileLabel  = new QLabel(tr("File:"),imgGroup);
  QLabel *imgTitleLabel = new QLabel(tr("Title:"),imgGroup);
  QLabel *imgIdLabel    = new QLabel(tr("Id:"),imgGroup);
  QLabel *imgInfoLabel  = new QLabel(imgInfoMsg,imgGroup);
  QLabel *imgCounterLabel = new QLabel(tr("Counter:"),imgGroup);
  m_imgFileEdit         = new QLineEdit(imgGroup);
  m_imgTitleEdit        = new QLineEdit(imgGroup);
  m_imgIdEdit           = new QLineEdit(imgGroup);
  m_imgCounterEdit      = new QLineEdit(imgGroup);
  imgFileLabel->setBuddy(m_imgFileEdit);
  imgTitleLabel->setBuddy(m_imgTitleEdit);
  imgIdLabel->setBuddy(m_imgIdEdit);
  imgCounterLabel->setBuddy(m_imgCounterEdit);
  
  QGridLayout *imgLayout = new QGridLayout(imgGroup);
  imgLayout->setColumnStretch(0,0);
  imgLayout->setColumnStretch(1,1);
  imgLayout->addWidget(imgFileLabel,0,0);
  imgLayout->addWidget(m_imgFileEdit,0,1);
  imgLayout->addWidget(imgTitleLabel,1,0);
  imgLayout->addWidget(m_imgTitleEdit,1,1);
  imgLayout->addWidget(imgIdLabel,2,0);
  imgLayout->addWidget(m_imgIdEdit,2,1);
  imgLayout->addWidget(imgInfoLabel,3,1);
  imgLayout->addWidget(imgCounterLabel,4,0);
  imgLayout->addWidget(m_imgCounterEdit,4,1);
  imgLayout->setRowStretch(5,10);
  
  m_imgTitleEdit->setMaxLength(20);
  m_imgIdEdit->setMaxLength(4);

  QFont curFont = currentFont();
  m_imgTitleEdit->setFont(curFont);
  m_imgIdEdit->setFont(curFont);
  
  return imgGroup;
}

QWidget *Preferences::createFontGroup(QWidget *parent)
{
  // font group
  QGroupBox *fontGroup = new QGroupBox(tr("Image Browser Font"),parent);
  
  m_fontShiftedCheck = new QCheckBox(tr("Use shifted CBM Font"),fontGroup);
  QLabel *fontNameLabel = new QLabel(tr("Unshifted Font:"),fontGroup);
  m_fontNameEdit = new QLineEdit(fontGroup);
  m_fontSizeEdit = new QLineEdit(fontGroup);
  
  QLabel *fontShiftedNameLabel = new QLabel(tr("Shifted Font:"),fontGroup);
  m_fontShiftedNameEdit = new QLineEdit(fontGroup);
  m_fontShiftedSizeEdit = new QLineEdit(fontGroup);
  
  fontNameLabel->setBuddy(m_fontNameEdit);
  fontShiftedNameLabel->setBuddy(m_fontShiftedNameEdit);

  QToolButton *fontNameButton = new QToolButton(fontGroup);
  fontNameButton->setText("F");
  connect(fontNameButton,SIGNAL(clicked()),this,SLOT(onPickFont()));
  QToolButton *fontShiftedNameButton = new QToolButton(fontGroup);
  fontShiftedNameButton->setText("F");
  connect(fontShiftedNameButton,SIGNAL(clicked()),this,SLOT(onPickShiftedFont()));
  
  QGridLayout *fontLayout = new QGridLayout(fontGroup);
  fontLayout->addWidget(m_fontShiftedCheck,0,1,1,3);
  fontLayout->addWidget(fontNameLabel,1,0);
  fontLayout->addWidget(m_fontNameEdit,1,1);
  fontLayout->addWidget(m_fontSizeEdit,1,2);
  fontLayout->addWidget(fontNameButton,1,3);
  fontLayout->addWidget(fontShiftedNameLabel,2,0);
  fontLayout->addWidget(m_fontShiftedNameEdit,2,1);
  fontLayout->addWidget(m_fontShiftedSizeEdit,2,2);
  fontLayout->addWidget(fontShiftedNameButton,2,3);
  fontLayout->setRowStretch(3,10);
  fontLayout->setColumnStretch(0,0);
  fontLayout->setColumnStretch(1,5);
  fontLayout->setColumnStretch(2,1);
  fontLayout->setColumnStretch(3,0);
  
  return fontGroup;
}

void Preferences::onPickFont()
{
  bool shifted;
  QFont font,shiftedFont;
  getFontDefaults(shifted,font,shiftedFont);
  bool ok;
  font = QFontDialog::getFont(&ok,font);
  if(ok) {
    m_fontNameEdit->setText(font.family());
    m_fontSizeEdit->setText(QString::number(font.pointSize()));
  }
}

void Preferences::onPickShiftedFont()
{
  bool shifted;
  QFont font,shiftedFont;
  getFontDefaults(shifted,font,shiftedFont);
  bool ok;
  shiftedFont = QFontDialog::getFont(&ok,shiftedFont);
  if(ok) {
    m_fontShiftedNameEdit->setText(shiftedFont.family());
    m_fontShiftedSizeEdit->setText(QString::number(shiftedFont.pointSize()));
  }
}

QWidget *Preferences::createSeparatorGroup(QWidget *parent)
{
  // separator group
  QGroupBox *sepGroup = new QGroupBox(tr("Separator Templates"),parent);
  
  m_separatorModel = new QStringListModel();
  m_separatorView = new QListView(sepGroup);
  m_separatorView->setFont(currentFont());
  m_separatorView->setModel(m_separatorModel);

  QToolButton *addButton = new QToolButton(sepGroup);
  QToolButton *delButton = new QToolButton(sepGroup);
  addButton->setText("+");
  addButton->setFont(QFont("Courier"));
  delButton->setText("-");
  delButton->setFont(QFont("Courier"));
  connect(addButton,SIGNAL(clicked()),this,SLOT(onAddSeparator()));
  connect(delButton,SIGNAL(clicked()),this,SLOT(onDelSeparator()));
  
  QGridLayout *sepLayout = new QGridLayout(sepGroup);
  sepLayout->setColumnStretch(0,1);
  sepLayout->setColumnStretch(1,0);
  sepLayout->setRowStretch(0,0);
  sepLayout->setRowStretch(1,0);
  sepLayout->setRowStretch(2,1);
  sepLayout->addWidget(m_separatorView,0,0,3,1);
  sepLayout->addWidget(addButton,0,1);
  sepLayout->addWidget(delButton,1,1);

  return sepGroup;
}

void Preferences::onAddSeparator()
{
  QString newSep = tr("-NEW SEPARATOR-");
  int newPos = m_separatorModel->rowCount();
  m_separatorModel->insertRows(newPos,1);
  m_separatorModel->setData(m_separatorModel->index(newPos,0),newSep);
}

void Preferences::onDelSeparator()
{
  QModelIndexList modelIndexList = m_separatorView->selectionModel()->selectedRows();
  if(modelIndexList.size()!=1)
    return;
  QModelIndex removeIndex = modelIndexList[0];
  m_separatorModel->removeRows(removeIndex.row(),1);
}

QWidget *Preferences::createEmulatorGroup(QWidget *parent)
{
  // emulator group
  QString emuInfoMsg = "<small>" + tr("Argument Patterns:") + "<br>"
                       "<b>%i</b> " +tr("Disk Image Path") + "<br>"
                       "<b>%p</b> " +tr("Program Name (Unicode)") + "<br>"
                       "<b>%P</b> " +tr("Program Name (PETSCII)") + "<br>"
                       "</small>";
  QGroupBox *emuGroup = new QGroupBox(tr("External Commodore Emulator"),parent);
  QLabel *emuAppLabel       = new QLabel(tr("Application:"),emuGroup);
  QLabel *emuMountArgsLabel = new QLabel(tr("Mount Image:"),emuGroup);
  QLabel *emuRunArgsLabel   = new QLabel(tr("Run Program:"),emuGroup);
  QLabel *emuInfoLabel      = new QLabel(emuInfoMsg,emuGroup);
  m_emuAppEdit              = new QLineEdit(emuGroup);
  m_emuMountArgsEdit        = new QLineEdit(emuGroup);
  m_emuRunArgsEdit          = new QLineEdit(emuGroup);
  emuAppLabel->setBuddy(m_emuAppEdit);
  emuMountArgsLabel->setBuddy(m_emuMountArgsEdit);
  emuRunArgsLabel->setBuddy(m_emuRunArgsEdit);
  
  QToolButton *emuAppButton = new QToolButton(emuGroup);
  emuAppButton->setText("...");
  connect(emuAppButton,SIGNAL(clicked()),this,SLOT(onPickEmuApp()));

  QGridLayout *emuLayout = new QGridLayout(emuGroup);
  emuLayout->setColumnStretch(0,0);
  emuLayout->setColumnStretch(1,1);
  emuLayout->addWidget(emuAppLabel,0,0);
  emuLayout->addWidget(m_emuAppEdit,0,1);
  emuLayout->addWidget(emuAppButton,0,2);
  emuLayout->addWidget(emuMountArgsLabel,1,0);
  emuLayout->addWidget(m_emuMountArgsEdit,1,1,1,2);
  emuLayout->addWidget(emuRunArgsLabel,2,0);
  emuLayout->addWidget(m_emuRunArgsEdit,2,1,1,2);
  emuLayout->addWidget(emuInfoLabel,3,1,1,2);
  emuLayout->setRowStretch(4,10);

  return emuGroup;
}

void Preferences::onPickEmuApp()
{
  QString oldPath = m_emuAppEdit->text();
  QString newPath = QFileDialog::getOpenFileName(this,
                                                 tr("Pick Emulator Application"),
                                                 oldPath,QString(),0,
                                                 QFileDialog::DontResolveSymlinks);
  if(newPath!="") {
    m_emuAppEdit->setText(newPath);
  }
}

QWidget *Preferences::createNetworkGroup(QWidget *parent)
{
  // network group
  QGroupBox *netGroup = new QGroupBox(tr("Network Settings"),parent);
  QLabel *netMyAddrLabel    = new QLabel(tr("My Address:"),netGroup);
  QLabel *netC64AddrLabel   = new QLabel(tr("C64 Address:"),netGroup);
  m_netMyAddrEdit           = new QLineEdit(netGroup);
  m_netC64AddrEdit          = new QLineEdit(netGroup);
  netMyAddrLabel->setBuddy(m_netMyAddrEdit);
  netC64AddrLabel->setBuddy(m_netC64AddrEdit);
  m_netMyAddrEdit->setInputMask("000.000.000.000");
  m_netC64AddrEdit->setInputMask("000.000.000.000");

  // warp copy
  QLabel *wcPrgLabel = new QLabel(tr("WarpCopy PRG File:"),netGroup);
  m_netWarpCopyPrgEdit = new QLineEdit(netGroup);
  m_netWarpCopyPatchCheck = new QCheckBox(tr("Patch WarpCopy IPs"),netGroup);

  QToolButton *wcPrgButton = new QToolButton(netGroup);
  wcPrgButton->setText("...");
  connect(wcPrgButton,SIGNAL(clicked()),this,SLOT(onPickWcPrg()));
  
  QGridLayout *netLayout = new QGridLayout(netGroup);
  netLayout->setColumnStretch(0,0);
  netLayout->setColumnStretch(1,1);
  netLayout->addWidget(netMyAddrLabel,0,0);
  netLayout->addWidget(m_netMyAddrEdit,0,1,1,2);
  netLayout->addWidget(netC64AddrLabel,1,0);
  netLayout->addWidget(m_netC64AddrEdit,1,1,1,2);
  netLayout->setRowStretch(2,1);
  netLayout->addWidget(wcPrgLabel,3,0);
  netLayout->addWidget(m_netWarpCopyPrgEdit,3,1);
  netLayout->addWidget(wcPrgButton,3,2);
  netLayout->addWidget(m_netWarpCopyPatchCheck,4,1,1,2);
  netLayout->setRowStretch(5,5);
  
  return netGroup;
}

void Preferences::onPickWcPrg()
{
  QString oldPath = m_netWarpCopyPrgEdit->text();
  QString newPath = QFileDialog::getOpenFileName(this,
                                                 tr("Pick WarpCopy PRG"),
                                                 oldPath,QString(),0,
                                                 QFileDialog::DontResolveSymlinks);
  if(newPath!="") {
    m_netWarpCopyPrgEdit->setText(newPath);
  }
}

// ----- load/save -----

void Preferences::load()
{
  // image
  QString file,title,id;
  int counter;
  getImageDefaults(file,title,id,counter);
  m_imgFileEdit->setText(file);
  m_imgTitleEdit->setText(title);
  m_imgIdEdit->setText(id);
  m_imgCounterEdit->setText(QString::number(counter));
  
  // font
  bool shifted;
  QFont font,shiftedFont;
  getFontDefaults(shifted,font,shiftedFont);
  m_fontShiftedCheck->setChecked(shifted);
  m_fontNameEdit->setText(font.family());
  m_fontSizeEdit->setText(QString::number(font.pointSize()));
  m_fontShiftedNameEdit->setText(shiftedFont.family());
  m_fontShiftedSizeEdit->setText(QString::number(shiftedFont.pointSize()));
  
  // separator
  QStringList templates;
  getSeparatorDefaults(templates);
  m_separatorModel->setStringList(templates);
  
  // emulator
  QString app,mountArgs,runArgs;
  getEmulatorDefaults(app,mountArgs,runArgs);
  m_emuAppEdit->setText(app);
  m_emuMountArgsEdit->setText(mountArgs);
  m_emuRunArgsEdit->setText(runArgs);
  
  // network
  AddrPair addrPair;
  getNetworkDefaults(addrPair);
  m_netMyAddrEdit->setText(addrPair.myAddr.toString());
  m_netC64AddrEdit->setText(addrPair.c64Addr.toString());
  
  // warp copy
  QString wcPrg;
  bool wcPatch;
  getWarpCopyDefaults(wcPrg,wcPatch);
  m_netWarpCopyPrgEdit->setText(wcPrg);
  m_netWarpCopyPatchCheck->setChecked(wcPatch);
}

void Preferences::save()
{
  // image
  QString file = m_imgFileEdit->text();
  QString title = m_imgTitleEdit->text();
  QString id = m_imgIdEdit->text();
  int counter = m_imgIdEdit->text().toInt();
  setImageDefaults(file,title,id,counter);
  
  // font
  bool shifted = m_fontShiftedCheck->isChecked();
  QFont font(m_fontNameEdit->text(),
             m_fontSizeEdit->text().toInt());
  QFont shiftedFont(m_fontShiftedNameEdit->text(),
                    m_fontShiftedSizeEdit->text().toInt());
  setFontDefaults(shifted,font,shiftedFont);
  
  // separator
  QStringList templates = m_separatorModel->stringList();
  setSeparatorDefaults(templates);
  
  // emulator
  QString app = m_emuAppEdit->text();
  QString mountArgs = m_emuMountArgsEdit->text();
  QString runArgs = m_emuRunArgsEdit->text();
  setEmulatorDefaults(app,mountArgs,runArgs);
  
  // network
  AddrPair addrPair;
  addrPair.myAddr = QHostAddress(m_netMyAddrEdit->text());
  addrPair.c64Addr = QHostAddress(m_netC64AddrEdit->text());
  setNetworkDefaults(addrPair);
  
  // warp copy
  QString wcPrg = m_netWarpCopyPrgEdit->text();
  bool wcPatch = m_netWarpCopyPatchCheck->isChecked();
  setWarpCopyDefaults(wcPrg,wcPatch);
}  

// ----- Defaults -----

// image

void Preferences::getImageDefaults(QString &file,
                                   QString &title,
                                   QString &id,
                                   int &counter)
{
  QSettings settings;
  settings.beginGroup("Preferences/Image");
  file    = settings.value("File","image%3c.d64").toString();
  title   = settings.value("Title","IMAGE %3C").toString();
  id      = settings.value("Id","%2C").toString();
  counter = settings.value("Counter",0).toInt();
  settings.endGroup();
}

void Preferences::setImageDefaults(const QString &file,
                                   const QString &title,
                                   const QString &id,
                                   int counter)
{
  QSettings settings;
  settings.beginGroup("Preferences/Image");
  settings.setValue("File",file);
  settings.setValue("Title",title);
  settings.setValue("Id",id);
  settings.setValue("Counter",counter);
  settings.endGroup();  
}

void Preferences::getNextImageName(QString &file,QString &title,QString &id)
{
  QString f,t,i;
  int counter;
  getImageDefaults(f,t,i,counter);
  
  bool usedCounter = false;
  if(insertCounter(f,counter,file))
    usedCounter = true;
  if(insertCounter(t,counter,title))
    usedCounter = true;
  if(insertCounter(i,counter,id))
    usedCounter = true;

  // ensure id size
  if(id.isEmpty()) {
    id.resize(2);
    id[0]=' ';
    id[1]=' ';
  } else if(id.size()==1) {
    id.resize(2);
    id[1]=' ';
  }
  if(id.size()>2) {
    id.resize(2);
  }
  // ensure title size
  if(title.size()>16) {
    title.resize(16);
  }

  if(usedCounter) {
    counter++;
    setImageDefaults(f,t,i,counter);
  }
}

bool Preferences::insertCounter(const QString &pattern,int counter,QString &result)
{
  bool counterFound = false;
  int size = pattern.size();
  for(int i=0;i<size;i++) {
    bool replaced = false;
    // is a % marker?
    if(pattern[i]=='%') {
      if(i<(size-1)) {
        QChar nextChar = pattern[i+1];
        if(nextChar.isDigit()) {
          int padding = pattern[i+1].toLatin1() - '0';
          if(i<(size-2)){
            QChar nextNextChar = pattern[i+2];
            if((nextNextChar=='c')||(nextNextChar=='C')) {
              // replace with padding
              replaced = true;
              counterFound = true;
              i+=2;
              QString line = "0000000000" + QString::number(counter);
              result += line.right(padding); 
            }
          }
        }
        else if((nextChar=='c')||(nextChar=='C')) {
          // replace without padding
          replaced = true;
          counterFound = true;
          i++;
          result += QString::number(counter);
        }
        // quoted %
        else if(nextChar=='%') {
          result += nextChar;
          i++;
          replaced = true;
        }
      }
    }
    if(!replaced) {
      result += pattern[i];
    }
  }
  return counterFound;
}

// emulator

void Preferences::getEmulatorDefaults(QString &app,
                                      QString &mountArgs,
                                      QString &runArgs)
{  
  QSettings settings;
  settings.beginGroup("Preferences/Emulator");
  app       = settings.value("App","x64").toString();
  mountArgs = settings.value("MountArgs","%i").toString();
  runArgs   = settings.value("RunArgs","%i:%p").toString();
  settings.endGroup();
}

void Preferences::setEmulatorDefaults(const QString &app,
                                      const QString &mountArgs,
                                      const QString &runArgs)
{  
  QSettings settings;
  settings.beginGroup("Preferences/Emulator");
  settings.setValue("App",app);
  settings.setValue("MountArgs",mountArgs);
  settings.setValue("RunArgs",runArgs);
  settings.endGroup();
}

// font

void Preferences::getFontDefaults(bool &shifted,
                                  QFont &font,QFont &shiftedFont)
{
  QFont defFont("CBM",8);
  QFont defShiftedFont("CBMShift",8);
  
  QSettings settings;
  settings.beginGroup("Preferences/Font");
  shifted       = settings.value("Shifted",false).toBool();
  font          = settings.value("Font",defFont).value<QFont>();
  shiftedFont   = settings.value("ShiftedFont",defShiftedFont).value<QFont>();
  settings.endGroup();
}

void Preferences::setFontDefaults(bool shifted,
                                  const QFont &font,const QFont &shiftedFont)
{
  QSettings settings;
  settings.beginGroup("Preferences/Font");
  settings.setValue("Shifted",shifted);
  settings.setValue("Font",font);
  settings.setValue("ShiftedFont",shiftedFont);
  settings.endGroup();
}

QFont Preferences::currentFont()
{
  QFont font,shiftedFont;
  bool shifted;
  getFontDefaults(shifted,font,shiftedFont);
  return shifted ? shiftedFont : font;  
}

// separator

void Preferences::getSeparatorDefaults(QStringList &templates)
{
  // create default separators
  QStringList defSeparators;
  const QChar defChars[5] = { '-','=','+','*','\x60' };
  for(int i=0;i<5;i++) {
    QString sep;
    for(int j=0;j<16;j++)
      sep += defChars[i];
    defSeparators << sep;
  }

  QSettings settings;
  settings.beginGroup("Preferences/Separator");
  templates = settings.value("Templates",defSeparators).toStringList();
  settings.endGroup();
}

void Preferences::setSeparatorDefaults(const QStringList &templates)
{
  QSettings settings;
  settings.beginGroup("Preferences/Separator");
  settings.setValue("Templates",templates);
  settings.endGroup();
}

// network

void Preferences::getNetworkDefaults(AddrPair &addrPair)
{
  QSettings settings;
  settings.beginGroup("Preferences/Network");
  QString ma = settings.value("MyAddr","192.168.64.1").toString();
  addrPair.myAddr = QHostAddress(ma);
  addrPair.myPort = (quint16)settings.value("MyPort",0).toUInt();
  QString ca = settings.value("C64Addr","192.168.64.2").toString();
  addrPair.c64Addr = QHostAddress(ca);
  addrPair.c64Port = (quint16)settings.value("C64Port",0).toUInt();
  settings.endGroup();
}

void Preferences::setNetworkDefaults(const AddrPair &addrPair)
{
  QSettings settings;
  settings.beginGroup("Preferences/Network");
  settings.setValue("MyAddr",addrPair.myAddr.toString());
  settings.setValue("MyPort",addrPair.myPort);
  settings.setValue("C64Addr",addrPair.c64Addr.toString());
  settings.setValue("C64Port",addrPair.c64Port);
  settings.endGroup();
}

void Preferences::getWarpCopyDefaults(QString &prg,bool &patch)
{
  QSettings settings;
  settings.beginGroup("Preferences/WarpCopy");
  prg = settings.value("prg","WARPCOPY06.PRG").toString();
  patch = settings.value("patch",true).toBool();
  settings.endGroup();
}

void Preferences::setWarpCopyDefaults(const QString &prg,bool patch)
{
  QSettings settings;
  settings.beginGroup("Preferences/WarpCopy");
  settings.setValue("prg",prg);
  settings.setValue("patch",patch);
  settings.endGroup();
}
