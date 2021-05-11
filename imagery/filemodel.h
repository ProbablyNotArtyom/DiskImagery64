#ifndef FILEMODEL_H
#define FILEMODEL_H

#include <QtGui/QtGui>
#include "cbmfile.h"

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
#include <QDirModel>

class FileModel : public QDirModel {
	Q_OBJECT

public:
	FileModel(QObject *parent=nullptr);
	~FileModel();

	Qt::ItemFlags flags(const QModelIndex &) const;

	bool fileForIndex(const QModelIndex &index, CBMFile &file) const;

	// change image
	QMimeData *copySelection(const QModelIndexList &indexes) const;
	bool pasteSelection(const QMimeData *data, const QModelIndex &parent);
	bool deleteSelection(const QModelIndexList &indexes);

	// drag & drop support
	virtual Qt::DropActions supportedDragActions() const { return Qt::CopyAction; }
	QStringList mimeTypes () const;
	QMimeData *mimeData(const QModelIndexList &indexes) const;
	bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
};

#endif

