#include <QAbstractItemView>
#include <QAction>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QImage>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QTreeView>
#include <QVBoxLayout>
#include <QWidget>

#include "dimagewin.h"
#include "petscii.h"
#include "preferences.h"

DImageWin::DImageWin(DImage::DiskFormat format, QWidget *parent)
    : MainWin("DImageWin", QRect(300, 100, 200, 300), parent),
    m_dimage(format) {

    QString file, title, id;
    Preferences::getNextImageName(file, title, id);
    m_dimage.setFileName(file);
    m_dimage.format(title, id);
    m_dimage.setDirty(false);

    init();
}

DImageWin::DImageWin(const QString &fileName, QWidget *parent)
    : MainWin("DImageWin", QRect(300, 100, 200, 300), parent),
    m_dimage(fileName) {
    init();
}

DImageWin::~DImageWin() {
    delete m_charsetDialog;
    delete m_formatImageDialog;
    delete m_addSeparatorDialog;
}

void DImageWin::init() {
    QWidget *mainWidget = new QWidget(this);
    setCentralWidget(mainWidget);
    QGridLayout *layout = new QGridLayout(mainWidget);
    layout->setMargin(2);
    layout->setSpacing(2);

    // disk title label
    m_diskTitle = new QLabel(mainWidget);
    m_diskTitle->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    layout->addWidget(m_diskTitle, 1, 0, 1, 2);

    // directory view
    m_model = new DImageModel(&m_dimage);
    m_dirView = new QTreeView(mainWidget);
    m_dirView->setModel(m_model);
    m_dirView->setDragDropMode(QAbstractItemView::DragDrop);
    m_dirView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_dirView->header()->setStretchLastSection(true);
    m_dirView->setRootIsDecorated(false);
    m_origFont = m_dirView->font().toString();
    layout->addWidget(m_dirView, 2, 0);
    layout->setRowStretch(2, 1);

    // blocks free and drive status labels
    m_blocksFree = new QLabel(mainWidget);
    m_driveStatus = new QLabel(mainWidget);
    m_driveStatus->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    layout->addWidget(m_blocksFree, 3, 0, 1, 2);
    layout->addWidget(m_driveStatus, 4, 0, 1, 2);

    // setup icon
    m_fileIcon = style()->standardIcon(QStyle::SP_FileIcon, nullptr, this);
    m_darkFileIcon = QIcon(darkenPixmap(m_fileIcon.pixmap(16, 16)));

    QFont font, shiftedFont;
    Preferences::getFontDefaults(m_fontShifted, font, shiftedFont);
    QColor fgColor, bgColor;
    Preferences::getColorDefaults(fgColor, bgColor);
    updateFont();
    updateDImage();
    connect(m_model, SIGNAL(changedDImage()), this, SLOT(updateDImage()));

    // connect menu entries
    connect(m_saveImageAction, SIGNAL(triggered()), this, SLOT(saveImage()));
    connect(m_saveImageAsAction, SIGNAL(triggered()), this,
        SLOT(saveImageAs()));

    connect(m_cutAction, SIGNAL(triggered()), this, SLOT(cut()));
    connect(m_copyAction, SIGNAL(triggered()), this, SLOT(copy()));
    connect(m_pasteAction, SIGNAL(triggered()), this, SLOT(paste()));
    connect(m_deleteAction, SIGNAL(triggered()), this, SLOT(deleteSel()));

    connect(m_shiftCharsetAction, SIGNAL(toggled(bool)), this,
        SLOT(shiftCharset(bool)));
    connect(m_showCharsetAction, SIGNAL(triggered()), this,
        SLOT(showCharset()));

    connect(m_formatDiskAction, SIGNAL(triggered()), this, SLOT(formatDisk()));
    connect(m_addSeparatorAction, SIGNAL(triggered()), this,
        SLOT(addSeparator()));
    connect(m_mountImageAction, SIGNAL(triggered()), this, SLOT(mountImage()));
    connect(m_runProgramAction, SIGNAL(triggered()), this, SLOT(runProgram()));

    connect(m_dirView, SIGNAL(activated(const QModelIndex &)), this,
        SLOT(activateItem(const QModelIndex &)));

    // init dialog
    m_charsetDialog = nullptr;
    m_formatImageDialog = nullptr;
    m_addSeparatorDialog = nullptr;
}

QPixmap DImageWin::darkenPixmap(const QPixmap &pixmap) {
    QPixmap newPixmap(pixmap);
    // QPixmap alpha = pixmap.alphaChannel();
    {
        QPainter painter(&newPixmap);
        painter.fillRect(pixmap.rect(), QBrush(QColor(0, 0, 0, 128)));
    }
    // newPixmap.setAlphaChannel(alpha);
    return newPixmap;
}

QFont DImageWin::currentFont() {
    bool shifted;
    QFont font, shiftedFont;
    Preferences::getFontDefaults(shifted, font, shiftedFont);
    return m_fontShifted ? shiftedFont : font;
}

QColor DImageWin::currentFGColor() {
    QColor fgColor, bgColor;
    Preferences::getColorDefaults(fgColor, bgColor);
    return fgColor;
}

QColor DImageWin::currentBGColor() {
    QColor fgColor, bgColor;
    Preferences::getColorDefaults(fgColor, bgColor);
    return bgColor;
}

void DImageWin::updateFont() {
    QFont curFont = currentFont();
    QColor curFG = currentFGColor();
    QColor curBG = currentBGColor();

    QPalette p;
    p.setColor(QPalette::Foreground, curFG);
    p.setColor(QPalette::Text, curFG);
    p.setColor(QPalette::WindowText, curFG);
    p.setColor(QPalette::Base, curBG);
    p.setColor(QPalette::Background, curBG);
    p.setColor(QPalette::Window, curBG);
    p.setColor(QPalette::NoRole, curBG);
    p.setColor(QPalette::Light, curBG);
    p.setColor(QPalette::Dark, curBG);

    m_dirView->setFont(curFont);
    m_blocksFree->setFont(curFont);
    m_driveStatus->setFont(curFont);
    m_diskTitle->setFont(curFont);

    m_dirView->setPalette(p);
    m_blocksFree->setPalette(p);
    m_driveStatus->setPalette(p);
    m_diskTitle->setPalette(p);

    m_dirView->setForegroundRole(QPalette::Foreground);
    m_blocksFree->setForegroundRole(QPalette::Foreground);
    m_driveStatus->setForegroundRole(QPalette::Foreground);
    m_diskTitle->setForegroundRole(QPalette::Foreground);
    m_dirView->setBackgroundRole(QPalette::Background);
    m_blocksFree->setBackgroundRole(QPalette::Background);
    m_driveStatus->setBackgroundRole(QPalette::Background);
    m_diskTitle->setBackgroundRole(QPalette::Background);

#ifdef Q_OS_WIN32
    // restore font in list view headers
    curFont.fromString(m_origFont);
    m_dirView->header()->setFont(curFont);
#endif

    // set actions
    if (m_fontShifted != m_shiftCharsetAction->isChecked())
        m_shiftCharsetAction->setChecked(m_fontShifted);
}

void DImageWin::updateDImage() {
    // set window title and icon
    bool isDirty = m_dimage.isDirty();
    if (m_dimage.isFileAvailable()) {
        setWindowIcon(isDirty ? m_darkFileIcon : m_fileIcon);
    } else {
        setWindowIcon(QIcon(":/imagery/imagery-16.png"));
    }
    QString title = QFileInfo(m_dimage.fileName()).fileName();
    if (isDirty) title += " *";
#ifdef Q_OS_WIN32
    title = "DiskImagery64 - " + title;
#endif
    setWindowTitle(title);

    QString blank("                ");

    // show disk id
    QString diskName, diskId;
    m_dimage.diskTitle(diskName, diskId);
    QString displayDiskName = Petscii::convertToDisplayString(diskName);
    QString displayDiskId = Petscii::convertToDisplayString(diskId);
    QString name = diskName + blank.left(16 - diskName.size());
    m_diskTitle->setText(name + " " + diskId);

    // show blocks free
    int blocksFree = m_dimage.blocksFree();
    QString bf = QString::number(blocksFree) + " BLOCKS FREE";
    m_blocksFree->setText(bf);
    m_driveStatus->setText(m_dimage.status());

    // update model
    m_model->updateDImage();
    m_dirView->resizeColumnToContents(0);
}

// ----- Slots --------------------------------------------------------------

// ----- File Menu -----

bool DImageWin::saveImage() {
    if (!m_dimage.isFileAvailable()) return saveImageAs();
    if (!m_dimage.isDirty()) return true;

    m_dimage.sync();
    updateDImage();
    return true;
}

bool DImageWin::saveImageAs() {
    QStringList filter;
    filter << DImage::fileFilterForDiskFormat(m_dimage.diskFormat());
    filter << tr("All Files (*.*)");
    QString fileName = QFileDialog::getSaveFileName(
        this, tr("Save Image as"), m_dimage.fileName(), filter.join(";;"));
    if (fileName != "") {
        m_dimage.setFileName(fileName);
        m_dimage.sync();
        updateDImage();
        return true;
    }
    return false;
}

void DImageWin::closeEvent(QCloseEvent *event) {
    if (m_dimage.isDirty()) {
        int result = QMessageBox::warning(
            this, tr("Unsaved Image"),
            tr("Disk Image '%1' is not saved!")
            .arg(QFileInfo(m_dimage.fileName()).fileName()),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
            QMessageBox::Save);
        if (result == QMessageBox::Save) {
            if (saveImage()) event->accept();
            else event->ignore();
        } else if (result == QMessageBox::Discard) {
            event->accept();
        } else {
            event->ignore();
        }
    } else event->accept();
}

// ----- Edit Menu -----

void DImageWin::cut() {
    copy();
    deleteSel();
}

void DImageWin::copy() {
    QItemSelectionModel *selModel = m_dirView->selectionModel();
    if (selModel->hasSelection()) {
        QMimeData *data = m_model->copySelection(selModel->selectedRows());
        if (data != nullptr) QApplication::clipboard()->setMimeData(data);
    }
}

void DImageWin::paste() {
    const QMimeData *data = QApplication::clipboard()->mimeData();
    if (data != nullptr) m_model->pasteSelection(data);
}

void DImageWin::deleteSel() {
    QItemSelectionModel *selModel = m_dirView->selectionModel();
    if (selModel->hasSelection()) m_model->deleteSelection(selModel->selectedRows());
}

// ----- View Menu -----

void DImageWin::shiftCharset(bool on) {
    m_fontShifted = on;
    updateFont();
    if (m_charsetDialog != nullptr) showCharset();
}

void DImageWin::showCharset() {
    if (m_charsetDialog == nullptr) {
        m_charsetDialog = new QDialog(this, Qt::Tool);
        m_charsetDialog->setWindowIcon(QIcon(":/imagery/imagery-16.png"));
        QVBoxLayout *layout = new QVBoxLayout(m_charsetDialog);
        m_charsetChars = new QLabel(m_charsetDialog);
        layout->addWidget(m_charsetChars, 0);
        layout->setMargin(2);
        m_charsetChars->setTextInteractionFlags(Qt::TextSelectableByMouse);
        m_charsetChars->setFocusPolicy(Qt::ClickFocus);

        QString charset;
        for (int i = 0x20; i < 0x80; i++) {
            charset += QChar(i);
            if (i % 16 == 15) charset += "\n";
        }
        for (int i = 0xa0; i < 0x100; i++) {
            charset += QChar(i);
            if ((i % 16 == 15) && (i != 0xff)) charset += "\n";
        }
        m_charsetChars->setText(charset);
    }

    m_charsetDialog->setWindowTitle(m_fontShifted
        ? tr("Shifted CBM Charset")
        : tr("Unshifted CBM Charset"));

    m_charsetChars->setFont(currentFont());
    m_charsetDialog->show();
}

// ----- Tools Menu -----

void DImageWin::formatDisk() {
    if (m_formatImageDialog == nullptr) {
        m_formatImageDialog = new QDialog(this, Qt::Sheet);
        m_formatImageDialog->setWindowTitle(tr("Format Disk"));
        m_formatImageDialog->setWindowIcon(QIcon(":/imagery/imagery-16.png"));

        QLabel *nameLabel = new QLabel(tr("Disk Name:"), m_formatImageDialog);
        QLabel *idLabel = new QLabel(tr("Disk Id:"), m_formatImageDialog);
        m_formatImageName = new QLineEdit(m_formatImageDialog);
        m_formatImageId = new QLineEdit(m_formatImageDialog);
        nameLabel->setBuddy(m_formatImageName);
        idLabel->setBuddy(m_formatImageId);

        QGridLayout *layout = new QGridLayout(m_formatImageDialog);
        layout->setColumnStretch(0, 0);
        layout->setColumnStretch(1, 1);
        layout->addWidget(nameLabel, 0, 0);
        layout->addWidget(m_formatImageName, 0, 1);
        layout->addWidget(idLabel, 1, 0);
        layout->addWidget(m_formatImageId, 1, 1);

        QDialogButtonBox *box = new QDialogButtonBox(m_formatImageDialog);
        box->addButton(tr("&Format"), QDialogButtonBox::AcceptRole);
        box->addButton(tr("&Cancel"), QDialogButtonBox::RejectRole);
        connect(box, SIGNAL(accepted()), m_formatImageDialog, SLOT(accept()));
        connect(box, SIGNAL(rejected()), m_formatImageDialog, SLOT(reject()));
        layout->addWidget(box, 2, 0, 1, 2);

        // set validator for input
        m_formatImageName->setMaxLength(16);
        m_formatImageId->setMaxLength(2);
    }

    // always update font
    QFont curFont = currentFont();
    m_formatImageName->setFont(curFont);
    m_formatImageId->setFont(curFont);
    // preset values
    QString name, id;
    m_dimage.diskTitle(name, id);
    m_formatImageName->setText(name);
    m_formatImageId->setText(id);

    // open dialog
    if (m_formatImageDialog->exec() == QDialog::Accepted) {
        QString name = m_formatImageName->text();
        QString id = m_formatImageId->text();
        int idSize = id.size();
        if ((idSize != 0) && (idSize != 2)) {
            // invalid id
            QMessageBox::warning(this, tr("Format Image"),
                tr("Invalid disk id given!"));
        } else {
            // format image
            m_dimage.format(name, id);
            updateDImage();
        }
    }
}

void DImageWin::addSeparator() {
    if (m_addSeparatorDialog == nullptr) {
        m_addSeparatorDialog = new QDialog(this, Qt::Sheet);
        m_addSeparatorDialog->setWindowTitle(tr("Add Separator"));
        m_addSeparatorDialog->setWindowIcon(QIcon(":/imagery/imagery-16.png"));

        QLabel *sepLabel = new QLabel(tr("Separator:"), m_addSeparatorDialog);
        m_addSeparatorCombo = new QComboBox(m_addSeparatorDialog);
        m_addSeparatorCombo->setEditable(true);
        sepLabel->setBuddy(m_addSeparatorCombo);

        QGridLayout *layout = new QGridLayout(m_addSeparatorDialog);
        layout->setColumnStretch(0, 0);
        layout->setColumnStretch(1, 1);
        layout->addWidget(sepLabel, 0, 0);
        layout->addWidget(m_addSeparatorCombo, 0, 1);

        QDialogButtonBox *box = new QDialogButtonBox(m_addSeparatorDialog);
        box->addButton(tr("&Add"), QDialogButtonBox::AcceptRole);
        box->addButton(tr("&Cancel"), QDialogButtonBox::RejectRole);
        connect(box, SIGNAL(accepted()), m_addSeparatorDialog, SLOT(accept()));
        connect(box, SIGNAL(rejected()), m_addSeparatorDialog, SLOT(reject()));
        layout->addWidget(box, 1, 0, 1, 2);
    }

    // always update font
    QFont curFont = currentFont();
    m_addSeparatorCombo->setFont(curFont);
    m_addSeparatorCombo->clear();
    QStringList templates;
    Preferences::getSeparatorDefaults(templates);
    m_addSeparatorCombo->addItems(templates);
    m_addSeparatorCombo->view()->setFont(curFont);

    if (m_addSeparatorDialog->exec() == QDialog::Accepted) {
        QString sep = m_addSeparatorCombo->currentText();
        if (sep == "") {
            QMessageBox::warning(this, tr("Add Separator"), tr("Ignoring empty separator!"));
        } else {
            // add separator
            CBMFile sepFile(sep, CBMFile::DEL);
            m_dimage.writeFile(sepFile);
            updateDImage();
        }
    }
}

void DImageWin::mountImage() {
    if (!saveImage()) {
        QMessageBox::warning(
            this, tr("Mount Image"),
            tr("You need to save an Image before mounting it!"));
        return;
    }

    QString app, mount, run;
    Preferences::getEmulatorDefaults(app, mount, run);
    runEmu(app, mount);
}

void DImageWin::runProgram() {
    QModelIndexList selection = m_dirView->selectionModel()->selectedRows();
    if (selection.size() != 1) {
        QMessageBox::warning(this, tr("Run Program"),
            tr("You need to select a single Program before running it!"));
        return;
    }
    if (!saveImage()) {
        QMessageBox::warning(this, tr("Run Program"),
            tr("You need to save an Image before mounting it!"));
        return;
    }

    QString app, mount, run;
    Preferences::getEmulatorDefaults(app, mount, run);
    QString fileName = m_model->data(m_model->index(selection[0].row(), 0)).toString();
    runEmu(app, run, fileName);
}

void DImageWin::runEmu(const QString &app, const QString &args,
                       const QString &fileName) {
    // create command line
    QStringList cmdLine;
    foreach (QString arg, args.split(" ")) {
        QString narg = replaceArgTags(arg, fileName);
        cmdLine << narg;
    }

    if (!QProcess::startDetached(app, cmdLine)) {
        QMessageBox::warning(this, tr("Mount Image"),
            tr("Cannot run the emulator with:\n") + app +
            "\n" + cmdLine.join(" "));
    }
}

QString DImageWin::replaceArgTags(const QString &str, const QString &fileName) {
    QString result;
    int size = str.size();
    for (int i = 0; i < size; i++) {
        bool replaced = false;
        if (str[i] == '%') {
            if (i < (size - 1)) {
                QChar nextChar = str[i + 1];
                // image path name
                if (nextChar == 'i') {
                    result += m_dimage.fileName();
                    i++;
                    replaced = true;
                }
                // program name in petscii
                else if (nextChar == 'P') {
                    result += fileName;
                    i++;
                    replaced = true;
                }
                // program name in unicode/ascii
                else if (nextChar == 'p') {
                    result += Petscii::convertToAscii(fileName, Petscii::VALID_PATTERN);
                    i++;
                    replaced = true;
                }
                // quoted %
                else if (nextChar == '%') {
                    result += nextChar;
                    i++;
                    replaced = true;
                }
            }
        }
        if (!replaced) result += str[i];
    }
    return result;
}

bool DImageWin::getCurrentFile(CBMFile &file) {
    QModelIndexList selection = m_dirView->selectionModel()->selectedRows();

    if (selection.size() != 1) return false;
    if (!m_model->fileForIndex(selection[0], file)) return false;

    return m_dimage.readFile(file);
}

bool DImageWin::getCurrentFiles(CBMFileList &files) {
    QModelIndexList selection = m_dirView->selectionModel()->selectedRows();
    if (selection.empty()) {
        // read full directory of image
        m_dimage.readDirectory(files);
    } else {
        QString name, id;
        m_dimage.diskTitle(name, id);   // copy disk name
        files.setTitle(name, id);

        files.clear();                  // files from selection
        for (int i = 0; i < selection.size(); i++) {
            CBMFile file;
            if (!m_model->fileForIndex(selection[i], file)) return false;
            files << file;
        }
    }
    for (int i = 0; i < files.size(); i++)
        if (!m_dimage.readFile(files[i])) return false;

    return true;
}

void DImageWin::activateItem(const QModelIndex &index) {
    // click on first column edist name
    if (index.column() == 0) return;

    CBMFile file;
    if (!m_model->fileForIndex(index, file)) return;
    if (!m_dimage.readFile(file)) return;

    operateOnFile(file);
}
