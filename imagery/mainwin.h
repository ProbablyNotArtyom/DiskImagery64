#ifndef MAINWIN_H
#define MAINWIN_H

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

#include <QtGui/QtGui>
#include "preferences.h"
#include "netwin.h"
#include "cbmfile.h"

#include <QToolButton>
#include <QApplication>
#include <QMainWindow>

class MainWin : public QMainWindow
{
    Q_OBJECT

public:
    MainWin(const QString &winClass,const QRect &defGeo,QWidget *parent);
    ~MainWin();

    //! return current dimage
    virtual DImage *getEmbeddedDImage() = 0;

public slots:
    void newImage();
    void newBrowser();
    void openImage();

    void about();
    void preferences();

    void netRunProgram();
    void netShareFiles();
    void netShowLog();

    void netWCStartServer();
    void netWCWarpReadDisk();
    void netWCWarpWriteDisk();
    void netWCReadDisk();
    void netWCWriteDisk();
    void netWCFormatDisk();
    void netWCVerifyDisk();
    void netWCSendDOSCommand();
    void netWCGetDriveStatus();

protected slots:
    void lastWindowClosed();

protected:
    // File Menu
    QAction *m_newImageAction;
    QAction *m_newBrowserAction;
    QAction *m_openImageAction;
    QAction *m_saveImageAction;
    QAction *m_saveImageAsAction;
    QAction *m_closeAction;
    QAction *m_quitAction;
    // Edit Menu
    QAction *m_cutAction;
    QAction *m_copyAction;
    QAction *m_pasteAction;
    QAction *m_deleteAction;
    QAction *m_preferencesAction;
    // View Menu
    QAction *m_shiftCharsetAction;
    QAction *m_showCharsetAction;
    // Tools Menu
    QAction *m_formatDiskAction;
    QAction *m_addSeparatorAction;
    // Emulator Menu
    QAction *m_mountImageAction;
    QAction *m_runProgramAction;
    // Network Menu
    QAction *m_netRunProgramAction;
    QAction *m_netShareFilesAction;
    QAction *m_netShowLogAction;
    // WarpCopy
    QAction *m_netWCStartServerAction;
    QAction *m_netWCWarpReadDiskAction;
    QAction *m_netWCWarpWriteDiskAction;
    QAction *m_netWCReadDiskAction;
    QAction *m_netWCWriteDiskAction;
    QAction *m_netWCFormatDiskAction;
    QAction *m_netWCVerifyDiskAction;
    QAction *m_netWCSendDOSCommandAction;
    QAction *m_netWCGetDriveStatusAction;
    // Help Menu
    QAction *m_aboutAction;

    //! the single net win instance
    static NetWin *g_netWin;

    //! return current cbm file
    virtual bool getCurrentFile(CBMFile &file) = 0;
    //! return the selected or all files of an image
    virtual bool getCurrentFiles(CBMFileList &files) = 0;
    //! operate on a single file triggered by double click
    void operateOnFile(const CBMFile &file);

    //! run a file via CodeNet
    void netRunFile(const CBMFile &file);
    
private:
    QString m_winClass;
    QRect m_defGeo;

    // preferences dialog
    Preferences *m_preferencesDialog;

    void initActions();
    void initMenu();
    void saveSettings();
    void loadSettings();

    bool patchWarpCopy(QByteArray &array);
};

#endif
