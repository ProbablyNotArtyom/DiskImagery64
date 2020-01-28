#ifndef COPYDIALOG_H
#define COPYDIALOG_H

#include <QWidget>
#include <QDialog>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QHBoxLayout>
#include <QMessageBox>

#include "blockmapwidget.h"
#include "warpcopydisk.h"

class CopyDialog : public QDialog
{
    Q_OBJECT

public:
    CopyDialog(QWidget *parent=0);
    ~CopyDialog();

    bool copyDisk(int drive,WarpCopyService *service,
                  WarpCopyDisk::Mode mode,DImage *image);

protected slots:
    void abort();
    void procBlock(int track,int sector,int status,const QString &statusMsg);
    void prepareBlock(int track,int sector);
    void updateTime();

protected:
    QLabel *m_titleLabel;
    BlockMapWidget *m_blockMapWidget;
    QLabel *m_statusLabel;
    QLabel *m_timeLabel;
    QPushButton *m_abortButton;

    WarpCopyDisk *m_copier;

    QTimer m_timer;
    QDateTime m_startTime;
};

#endif

