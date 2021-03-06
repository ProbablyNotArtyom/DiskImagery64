#include "dimagemodel.h"
#include "cbmfile.h"
#include "petscii.h"

#include <QAbstractItemModel>
#include <QMessageBox>

//------------------------------------------------------------------------------

DImageModel::DImageModel(DImage *dimage) : m_dimage(dimage) {
	updateDImage();
}

DImageModel::~DImageModel() {}

Qt::ItemFlags DImageModel::flags(const QModelIndex &index) const {
	Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable |
	Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
	
	if (index.column() == 0) flags |= Qt::ItemIsEditable;
	
	return flags;
}

// drag support
QStringList DImageModel::mimeTypes() const {
	return CBMFileList::allMimeTypes();
}

QMimeData *DImageModel::copySelection(const QModelIndexList &indexes) const {
	CBMFileList files;
	
	// read all selected files
	foreach (const QModelIndex &index, indexes) {
		if (index.column() == 0) {
			int entry = index.row();
			CBMFile file = m_files.at(entry);
			if (m_dimage->readFile(file)) files << file;
		}
	}
	
	return files.convertToMimeData();		// convert to mime blob
}

bool DImageModel::fileForIndex(const QModelIndex &index, CBMFile &file) const {
	int pos = index.row();
	
	if (!index.isValid()) return false;
	if ((pos < 0) || (pos >= m_files.size())) return false;
	file = m_files.at(pos);
	return true;
}

QMimeData *DImageModel::mimeData(const QModelIndexList &indexes) const {
	return copySelection(indexes);
}

bool DImageModel::pasteSelection(const QMimeData *mimeData) {
	CBMFileList files;
	
	// extract file list from mime blob
	if (!files.parseFromMimeData(mimeData)) {
		qDebug("parsing failed!");
		return false;
	}
	
	// add all files to image
	QStringList errorFiles;
	for (int i = 0; i < files.size(); i++) {
		const CBMFile &file = files.at(i);
		if (!m_files.hasFile(file) || (file.type() == CBMFile::DEL)) {	// check if file is already there
			qDebug("write %d bytes to image", file.data().size());
			if (m_dimage->writeFile(file)) emit changedDImage();		// report change
			else errorFiles << (file.convertToAsciiName() + ": " + tr("Write Error"));
		} else errorFiles << (file.convertToAsciiName() + ": " + tr("Already Exists"));
	}
	
	// report errors while dropping
	if (!errorFiles.empty()) {
		QString message = tr("Erros occured:") + "\n" + errorFiles.join("\n");
		QMessageBox::warning(nullptr, tr("Error"), message);
	}
	
	return true;
}

bool DImageModel::dropMimeData(const QMimeData *mimeData,
	Qt::DropAction /*action*/, int /*row*/,
	int /*column*/, const QModelIndex & /*parent*/) {

	return pasteSelection(mimeData);
}

bool DImageModel::deleteSelection(const QModelIndexList &indexes) {
	bool allOk = true;
	foreach (const QModelIndex &index, indexes) {
		const CBMFile &file = m_files.at(index.row());
		bool ok = m_dimage->deleteFile(file);
		allOk = allOk && ok;
	}

	emit changedDImage();
	return allOk;
}

//------------------------------------------------------------------------------
// Extensions

void DImageModel::setFileRow(const CBMFile &file, int row) {
	QString name = Petscii::convertToDisplayString(file.name());
	QString blocks = QString("%1").arg(file.blocks());
	QString type = file.convertTypeFlagsToString();

	setData(index(row, 0), name);
	setData(index(row, 1), blocks);
	setData(index(row, 2), type);
}

bool DImageModel::setData(const QModelIndex &index, const QVariant &value, int role) {
	if ((role == Qt::EditRole) && (index.column() == 0)) {
		QString name = value.toString();
		CBMFile &file = m_files[index.row()];
		if (file.name() != name) {				// need to rename file on disk image!
			m_dimage->renameFile(file, name);
			return QStandardItemModel::setData(index, file.name(), role);
		}
	}
	return QStandardItemModel::setData(index, value, role);
}

void DImageModel::updateDImage() {
	m_files.clear();
	m_dimage->readDirectory(m_files);
	int size = m_files.size();
	
	clear();
	insertColumns(0, 3);
	insertRows(0, size);
	
	setHeaderData(0, Qt::Horizontal, tr("NAME"));
	setHeaderData(1, Qt::Horizontal, tr("BLOCKS"));
	setHeaderData(2, Qt::Horizontal, tr("TYPE"));
	
	
	for (int i = 0; i < size; i++) setFileRow(m_files.at(i), i);
}
