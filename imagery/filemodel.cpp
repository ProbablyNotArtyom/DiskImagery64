#include "filemodel.h"
#include "cbmfile.h"
#include "dimage.h"
#include "petscii.h"

#include <QDirModel>

FileModel::FileModel(QObject *parent) : QDirModel(parent) {
    // setSupportedDragActions(Qt::CopyAction);
    setReadOnly(false);
}

FileModel::~FileModel() {}

Qt::ItemFlags FileModel::flags(const QModelIndex &index) const {
    Qt::ItemFlags flags = QDirModel::flags(index);
    QFileInfo info = fileInfo(index);

    if (info.isDir()) flags |= Qt::ItemIsDropEnabled;
    else if (info.isFile()) {
        flags |= Qt::ItemIsDragEnabled;

        // do not edit disk image names but open them
        if (DImage::determineDiskFormat(filePath(index)) != DImage::INVALID)
            flags &= ~Qt::ItemIsEditable;
    }
    return flags;
}

QStringList FileModel::mimeTypes() const {
    QStringList mimeTypes;
    mimeTypes << CBMFileList::mimeType();
    return mimeTypes;
}

QMimeData *FileModel::copySelection(const QModelIndexList &indexes) const {
    CBMFileList files;

    // convert all selected files to cbm files
    foreach (const QModelIndex &index, indexes) {
        if (index.column() == 0) {
            CBMFile file;
            if (fileForIndex(index, file)) files << file;
        }
    }

    QMimeData *data = files.convertToMimeData();

    if (data != nullptr) {
        // add list of file urls
        QList<QUrl> urls;
        foreach (const QModelIndex &index, indexes) {
            QString fullPath = filePath(index);
            urls << QUrl::fromLocalFile(fullPath);
        }
        data->setUrls(urls);
    }

    return data;
}

bool FileModel::fileForIndex(const QModelIndex &index, CBMFile &cbmFile) const {
    if (!index.isValid()) return false;
    QString filePath = this->filePath(index);
    return cbmFile.fromLocalFile(filePath);
}

QMimeData *FileModel::mimeData(const QModelIndexList &indexes) const {
    return copySelection(indexes);
}

bool FileModel::pasteSelection(const QMimeData *data, const QModelIndex &parent) {
    CBMFileList files;

    // parse cbm files from mime blob
    if (!files.parseFromMimeData(data)) {
        qDebug("no cbm files dropped");
        return false;
    }
    // check drop directory
    if (!parent.isValid()) {
        qDebug("no valid index");
        return false;
    }

    QString dirPath = this->filePath(parent);
    QDir dir(dirPath);
    if (!dir.exists()) return false;

    // write files to directory
    QStringList errorFiles;
    for (int i = 0; i < files.size(); i++) {
        const CBMFile &file = files.at(i);
        QString fileName = file.convertToAsciiName();
        QString filePath = dir.filePath(fileName);
        if (!file.toLocalFile(filePath)) {
            errorFiles << tr("Can't store file '%1' in '%2'")
            .arg(file.name())
            .arg(dir.path());
        }
    }
    refresh(parent);

    // report errors while dropping
    if (!errorFiles.empty()) {
        QString message = tr("Erros occured:") + "\n" + errorFiles.join("\n");
        QMessageBox::warning(nullptr, tr("Error"), message);
    }

    return true;
}

bool FileModel::dropMimeData(const QMimeData *data, Qt::DropAction /*action*/,
    int /*row*/, int /*column*/, const QModelIndex &parent) {

    return pasteSelection(data, parent);
}

bool FileModel::deleteSelection(const QModelIndexList &indexes) {
    bool allOk = true;
    QSet<QModelIndex> refreshParents;
    QStringList deletePaths;
    foreach (const QModelIndex &index, indexes) {
        if (index.column() == 0) {
            QString filePath = this->filePath(index);
            QFileInfo info(filePath);
            // remove only files
            if (info.isFile()) {
                deletePaths << filePath;
                QModelIndex p = parent(index);
                if (p.isValid()) refreshParents << p;
            }
        }
    }
    foreach (QString path, deletePaths) {
        QModelIndex index = this->index(path);
        if (index.isValid()) {
            bool ok = remove(index);
            allOk = allOk && ok;
        }
    }
    foreach (const QModelIndex &index, refreshParents)
        refresh(index);

    return allOk;
}
