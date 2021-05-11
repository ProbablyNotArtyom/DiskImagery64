#include "filewin.h"
#include "dimagewin.h"

//------------------------------------------------------------------------------

FileWin::FileWin(const QString &rootDirName, QWidget *parent)
: MainWin("FileWin", QRect(100, 100, 200, 300), parent) {
	m_dirIcon = style()->standardIcon(QStyle::SP_DirOpenIcon, nullptr, this);

#ifdef Q_OS_WIN32
	setWindowTitle("DiskImagery64 - " + tr("File Browser"));
#else
	setWindowTitle(tr("File Browser"));
#endif
	setWindowIcon(m_dirIcon);

	QWidget *mainWidget = new QWidget(this);
	setCentralWidget(mainWidget);
	QGridLayout *layout = new QGridLayout(mainWidget);
	layout->setColumnStretch(0, 0);
	layout->setColumnStretch(1, 1);
	layout->setRowStretch(1, 1);
	layout->setMargin(2);
	layout->setSpacing(2);

	m_openDirButton = new QToolButton(mainWidget);
	m_openDirButton->setIcon(m_dirIcon);
	layout->addWidget(m_openDirButton, 0, 0);
	connect(m_openDirButton, SIGNAL(clicked()), this, SLOT(openDir()));

	m_currentDirEdit = new QLineEdit(mainWidget);
	layout->addWidget(m_currentDirEdit, 0, 1);
	connect(m_currentDirEdit, SIGNAL(returnPressed()), this, SLOT(newDirEntered()));

	m_model = new FileModel;
	m_model->setSorting(QDir::Name | QDir::DirsFirst | QDir::IgnoreCase | QDir::LocaleAware);
	m_view = new QTreeView(mainWidget);
	m_view->setModel(m_model);
	m_view->setDragEnabled(true);
	m_view->setAcceptDrops(true);
	m_view->header()->setStretchLastSection(true);
	m_view->setSelectionMode(QAbstractItemView::ExtendedSelection);
	layout->addWidget(m_view, 1, 0, 1, 2);

	// disable unused actions
	m_saveImageAction->setEnabled(false);
	m_saveImageAsAction->setEnabled(false);
	m_shiftCharsetAction->setEnabled(false);
	m_showCharsetAction->setEnabled(false);
	m_formatDiskAction->setEnabled(false);
	m_addSeparatorAction->setEnabled(false);
	m_mountImageAction->setEnabled(false);
	m_runProgramAction->setEnabled(false);
	m_netWCWriteDiskAction->setEnabled(false);
	m_netWCWarpWriteDiskAction->setEnabled(false);

	// connect used actions
	connect(m_cutAction, SIGNAL(triggered()), this, SLOT(cut()));
	connect(m_copyAction, SIGNAL(triggered()), this, SLOT(copy()));
	connect(m_pasteAction, SIGNAL(triggered()), this, SLOT(paste()));
	connect(m_deleteAction, SIGNAL(triggered()), this, SLOT(deleteSel()));

	// open disk images
	connect(m_view, SIGNAL(activated(const QModelIndex &)), this, SLOT(activateItem(const QModelIndex &)));

	if (rootDirName == "") loadSettings();
	else setRootDir(rootDirName);
}

FileWin::~FileWin() { saveSettings(); }

void FileWin::setRootDir(const QString &dirPath) {
	QString rootDirPath = dirPath;
	QDir rootDir(rootDirPath);
	if (!rootDir.exists()) rootDirPath = QDir::homePath();

	QModelIndex index = m_model->index(rootDirPath);
	m_view->setRootIndex(index);
	// m_view->expand(index);
	// m_view->scrollTo(index);
	m_view->resizeColumnToContents(0);
	m_currentDirEdit->setText(rootDirPath);
}

QString FileWin::rootDir() const { return m_model->filePath(m_view->rootIndex()); }

// ----- Slots -----

void FileWin::openDir() {
	QString dirPath = QFileDialog::getExistingDirectory(this, tr("Select Directory"), rootDir());
	if (dirPath != "") setRootDir(dirPath);
}

void FileWin::newDirEntered() { setRootDir(m_currentDirEdit->text()); }

void FileWin::activateItem(const QModelIndex &index) {
	QString filePath = m_model->filePath(index);
	QFileInfo info(filePath);
	if (info.exists()) {
		if (DImage::determineDiskFormat(filePath) != DImage::INVALID) {
			// open disk image
			DImageWin *win = new DImageWin(filePath);
			win->show();
		} else {
			// operate on file
			CBMFile file;
			if (m_model->fileForIndex(index, file)) operateOnFile(file);
		}
	}
}

void FileWin::cut() {
	copy();
	deleteSel();
}

void FileWin::copy() {
	QItemSelectionModel *selModel = m_view->selectionModel();
	if (selModel->hasSelection()) {
		QMimeData *data = m_model->copySelection(selModel->selectedRows());
		if (data != nullptr) QApplication::clipboard()->setMimeData(data);
	}
}

void FileWin::paste() {
	QModelIndex parent = m_view->currentIndex();
	if (!parent.isValid()) return;

	const QMimeData *data = QApplication::clipboard()->mimeData();
	if (data != nullptr) m_model->pasteSelection(data, parent);
}

void FileWin::deleteSel() {
	QItemSelectionModel *selModel = m_view->selectionModel();
	if (selModel->hasSelection()) m_model->deleteSelection(selModel->selectedRows());
}

bool FileWin::getCurrentFile(CBMFile &file) {
	QModelIndexList selection = m_view->selectionModel()->selectedRows();
	if (selection.size() != 1) return false;
	return m_model->fileForIndex(selection[0], file);
}

bool FileWin::getCurrentFiles(CBMFileList &files) {
	QModelIndexList selection = m_view->selectionModel()->selectedRows();
	if (selection.empty()) return false;

	files.setTitle("LOCAL HOST", "64");
	files.clear();
	foreach (QModelIndex index, selection) {
		CBMFile file;
		if (!m_model->fileForIndex(index, file)) return false;
		files << file;
	}
	return true;
}

// ----- Settings -----

void FileWin::loadSettings() {
	QSettings settings;
	settings.beginGroup("FileWin");
	QString rootDirName = settings.value("directory", QDir::homePath()).toString();
	setRootDir(rootDirName);
	settings.endGroup();
}

void FileWin::saveSettings() {
	QSettings settings;
	settings.beginGroup("FileWin");
	settings.setValue("directory", rootDir());
	settings.endGroup();
}
