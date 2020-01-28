#ifndef FILEWIN_H
#define FILEWIN_H

#include <QtGui/QtGui>
#include "filemodel.h"
#include "mainwin.h"

class FileWin : public MainWin
{
    Q_OBJECT

public:
    FileWin(const QString &rootDirName="",QWidget *parent=0);
    ~FileWin();

    virtual DImage *getEmbeddedDImage() { return 0; }

public slots:
    void openDir();
    void newDirEntered();
    void cut();
    void copy();
    void paste();
    void deleteSel();

protected slots:
    void activateItem(const QModelIndex &index);

protected:
    FileModel *m_model;
    QTreeView *m_view;

    QIcon m_dirIcon;
    QToolButton *m_openDirButton;
    QLineEdit *m_currentDirEdit;

    void loadSettings();
    void saveSettings();
    void setRootDir(const QString &dirPath);
    QString rootDir() const;

    //! return current cbm file
    virtual bool getCurrentFile(CBMFile &file);
    //! return current cbm files
    virtual bool getCurrentFiles(CBMFileList &files);
};

#endif
