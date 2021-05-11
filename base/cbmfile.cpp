#include "cbmfile.h"
#include "petscii.h"

// ----- CBMFile -----

CBMFile::CBMFile(const QString &name, Type type, const QByteArray &data)
	: m_name(name), m_type(type), m_data(data), m_closed(true), m_locked(false),
	m_blocks(-1) {}

CBMFile::CBMFile(const QString &name, Type type)
	: m_name(name), m_type(type), m_closed(true), m_locked(false),
	m_blocks(-1) {}

CBMFile::CBMFile(const CBMFile &file)
	: m_name(file.m_name), m_type(file.m_type), m_data(file.m_data),
	m_closed(file.m_closed), m_locked(file.m_locked),
	m_blocks(file.m_blocks) {}

CBMFile::~CBMFile() {}

QString CBMFile::convertTypeToString() const {
	const char *types[8] = {"DEL", "SEQ", "PRG", "USR", "REL", "CBM", "DIR", "???"};
	return types[m_type];
}

bool CBMFile::parseTypeFromString(const QString &typeName) {
	if (typeName == "DEL") m_type = DEL;
	else if (typeName == "SEQ") m_type = SEQ;
	else if (typeName == "PRG") m_type = PRG;
	else if (typeName == "USR") m_type = USR;
	else if (typeName == "REL") m_type = REL;
	else if (typeName == "CBM") m_type = CBM;
	else if (typeName == "DIR") m_type = DIR;
	else if (typeName == "???") m_type = UNKNOWN;
	else return false;
	return true;
}

QString CBMFile::convertTypeFlagsToString() const {
	QChar closed = m_closed ? QChar(' ') : QChar('*');
	QChar locked = m_locked ? QChar('<') : QChar(' ');
	return closed + convertTypeToString() + locked;
}

bool CBMFile::parseTypeFlagsFromString(const QString &str) {
	if (str.size() != 5) return false;

	QChar closedFlag = str[0];
	if (closedFlag == ' ') m_closed = true;
	else if (closedFlag == '*') m_closed = false;
	else return false;

	QChar lockedFlag = str[4];
	if (lockedFlag == '<') m_locked = true;
	else if (lockedFlag == ' ') m_locked = false;
	else return false;

	QString typeName = str.mid(1, 3);
	return parseTypeFromString(typeName);
}

QString CBMFile::convertToAsciiName() const {
	QString asciiName = Petscii::convertToAscii(m_name, Petscii::VALID_FILENAME);
	QString ext = convertTypeFlagsToString().mid(1, 3);
	QString asciiExt = Petscii::convertToAscii(ext, Petscii::VALID_FILENAME);
	return asciiName + "." + asciiExt;
}

void CBMFile::parseFromAsciiName(const QString &fileName, Type defType) {
	QString pName = Petscii::convertFromAscii(fileName);
	int size = pName.size();
	if (size > 4) {
		if (pName[size - 4] == '.') {
			QString typeName = pName.right(3).toUpper();
			if (parseTypeFromString(typeName)) {
				m_name = pName.left(size - 4);
				return;
			}
		}
	}
	m_name = pName.left(16);
	m_type = defType;
}

bool CBMFile::prgRunAddress(quint16 &addr) const {
	if (type() != PRG) return false;
	if (m_data.size() < 2) return false;
	const char *data = m_data.constData();
	addr = quint16(data[1] << 8) | quint16(data[0]);
	return true;
}

bool CBMFile::prgData(QByteArray &data) const {
	if (type() != PRG) return false;
	if (m_data.size() < 2) return false;

	int size = m_data.size() - 2;
	data.resize(size);
	for (int i = 0; i < size; i++) data[i] = m_data[i + 2];
	return true;
}

void CBMFile::calcBlocks() { m_blocks = (m_data.size() + 253) / 254; }

bool CBMFile::fromLocalFile(const QString &filePath) {
	QFileInfo info(filePath);
	if (info.isFile() && info.exists() && info.isReadable()) {
		QFile file(filePath);
		if (file.open(QIODevice::ReadOnly)) {
			m_data = file.readAll();

			qDebug("read %d bytes from '%s'", m_data.size(), filePath.toLatin1().data());

			parseFromAsciiName(info.fileName());		// make cbm file
			return true;
		}
	}
	return false;
}

bool CBMFile::toLocalFile(const QString &filePath) const {
	QFile file(filePath);
	qint64 size = 0;
	if (file.open(QIODevice::WriteOnly)) {
		size = file.write(m_data);
		file.close();
	} else return false;

	qDebug("wrote %d bytes to '%s'", m_data.size(), filePath.toLatin1().data());

	return (size == m_data.size());
}

// ----- CBMFileList -----

CBMFileList::CBMFileList() : m_freeBlocks(0) {}
CBMFileList::~CBMFileList() {}
QString CBMFileList::mimeType() { return "application/x-cbmfiles"; }

QStringList CBMFileList::allMimeTypes() {
	QStringList mimeTypes;
	mimeTypes << mimeType() << "text/uri-list";
	return mimeTypes;
}

QMimeData *CBMFileList::convertToMimeData() {
	QByteArray buffer;
	QDataStream stream(&buffer, QIODevice::WriteOnly);
	int numFiles = this->size();
	
	stream << QString("CBMFileList");
	stream << m_name << m_id << numFiles;

	foreach (const CBMFile &file, *this) {
		QString typeFlags = file.convertTypeFlagsToString();
		stream << file.name() << typeFlags << file.data();
	}

	qDebug("CBMFileList: %d bytes mime data created for %d files", buffer.size(), numFiles);
	QMimeData *mimeData = new QMimeData;
	mimeData->setData(mimeType(), buffer);

	return mimeData;
}

bool CBMFileList::parseFromMimeData(const QMimeData *mimeData) {
	if (!mimeData->hasFormat(mimeType())) {		// allow urls
		if (mimeData->hasUrls()) return parseFromUrls(mimeData->urls());
		qDebug("CBMFileList: wrong mime type!");
		return false;
	}
	QByteArray buffer = mimeData->data(mimeType());
	qDebug("CBMFileList: %d bytes mime data read", buffer.size());
	QDataStream stream(&buffer, QIODevice::ReadOnly);

	QString tag;
	stream >> tag;
	if (tag != "CBMFileList") {
		qDebug("CBMFileList: wrong tag!");
		return false;
	}

	int size;
	stream >> m_name >> m_id >> size;
	for (int i = 0; i < size; i++) {
		QString name, typeFlags;
		QByteArray data;
		stream >> name >> typeFlags >> data;
		CBMFile file(name, CBMFile::PRG, data);
		if (!file.parseTypeFlagsFromString(typeFlags)) return false;
		(*this) << file;
	}

	return true;
}

bool CBMFileList::parseFromUrls(const QList<QUrl> &urls) {
	foreach (QUrl url, urls) {
		QString filePath = url.toLocalFile();
		CBMFile file;
		qDebug("CBMFileList: parseURL '%s'", filePath.toLatin1().data());
		if (!file.fromLocalFile(filePath)) return false;
		(*this) << file;
	}
	return true;
}

bool CBMFileList::hasFile(const CBMFile &file) const {
	foreach (const CBMFile &f, *this) {
		if ((f.name() == file.name()) && (f.type() == file.type())) return true;
	}
	return false;
}

void CBMFileList::setTitle(const QString &name, const QString &id) {
	m_name = name;
	m_id = id;
}

void CBMFileList::title(QString &name, QString &id) const {
	name = m_name;
	id = m_id;
}
