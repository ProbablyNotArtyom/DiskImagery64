#ifndef DIMAGEMODEL_H
#define DIMAGEMODEL_H

#include <QtGui/QtGui>
#include "dimage.h"

#include <QAbstractItemModel>

class DImageModel : public QStandardItemModel
{
    Q_OBJECT

public:
    DImageModel(DImage *diamge);
    ~DImageModel();

    Qt::ItemFlags flags(const QModelIndex &) const;

    void updateDImage();

    bool setData(const QModelIndex &index,
                 const QVariant &value,
                 int role = Qt::EditRole);

    bool fileForIndex(const QModelIndex &index,CBMFile &file) const;

    // change image
    QMimeData *copySelection(const QModelIndexList &indexes) const;
    bool pasteSelection(const QMimeData *data);
    bool deleteSelection(const QModelIndexList &indexes);

    // drag & drop support
    Qt::DropActions supportedDragActions() const { return Qt::CopyAction; }
    Qt::DropActions supportedDropActions() const { return Qt::CopyAction; }
    QStringList mimeTypes() const;
    QMimeData *mimeData(const QModelIndexList &indexes) const;
    bool dropMimeData(const QMimeData *data,Qt::DropAction action,
                      int row,int column,const QModelIndex &parent);

signals:
    void changedDImage();

protected:
    DImage *m_dimage;
    CBMFileList m_files;

    void setFileRow(const CBMFile &file,int row);
};

#endif
