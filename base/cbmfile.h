#ifndef CBMFILES_H
#define CBMFILES_H

#include <QtCore/QtCore>

class CBMFile {
public:
    //! file type
    enum Type { DEL=0,SEQ=1,PRG=2,USR=3,
                REL=4,CBM=5,DIR=6,UNKNOWN=7 };

    //! create a empty cbm file
    CBMFile(const QString &name="",Type type=PRG);
    //! create a cbm file with contents
    CBMFile(const QString &name,Type type,const QByteArray &data);
    //! copy constructor
    CBMFile(const CBMFile &file);
    //! free file
    ~CBMFile();

    //! get petscii name of file
    QString name() const { return m_name; }
    //! get type string of file
    Type type() const { return m_type; }
    //! set the petscii name
    void setName(const QString &name) { m_name = name; }
    //! set the type
    void setType(Type t) { m_type = t; }

    //! get file raw data
    const QByteArray &data() const { return m_data; }
    //! set the file raw data
    void setData(const QByteArray &data) { m_data = data; calcBlocks(); }

    //! get the closed flag
    bool isClosed() const { return m_closed; }
    //! get the locked flag
    bool isLocked() const { return m_locked; }
    //! set the closed flag
    void setClosed(bool on) { m_closed = on; }
    //! set the locked flag
    void setLocked(bool on) { m_locked = on; }

    //! set the size in blocks
    void setBlocks(int blocks) { m_blocks = blocks; }
    //! calc blocks
    void calcBlocks();
    //! get the size in blocks (-1 unknown)
    int blocks() const { return m_blocks; }

    //! convert the type and flags to string
    QString convertTypeFlagsToString() const;
    //! parse the type and flags from a string
    bool parseTypeFlagsFromString(const QString &str);

    //! convert file name to ascii
    QString convertToAsciiName() const;
    //! parse file name from ascii
    void parseFromAsciiName(const QString &fileName,Type defType=PRG);

    //! convert type name to string
    QString convertTypeToString() const;
    //! parse type from string
    bool parseTypeFromString(const QString &str);

    //! prg file run addr
    bool prgRunAddress(quint16 &adr) const;
    //! prg run data
    bool prgData(QByteArray &data) const;

    //! create cbm file from local file
    bool fromLocalFile(const QString &filePath);
    //! save cbm file to local file
    bool toLocalFile(const QString &filePath) const;

protected:
    QString m_name;
    Type m_type;
    QByteArray m_data;
    bool m_closed;
    bool m_locked;
    int m_blocks;
};

class CBMFileList : public QList<CBMFile>
{
public:
    //! create a new cbm files container
    CBMFileList();
    //! remove a cbm files container
    ~CBMFileList();

    //! set (optional) title (PETSCII)
    void setTitle(const QString &name,const QString &id);
    //! get title (PETSCII)
    void title(QString &name,QString &id) const;

    //! set (optional) free blocks
    void setFreeBlocks(int blocks) { m_freeBlocks = blocks; }
    //! get free blocks
    int freeBlocks() const { return m_freeBlocks; }

    //! check if a file is already in the list
    bool hasFile(const CBMFile &file) const;

    /** @name Mime Handling */
    //@{
    //! return the mime type for the cbm files
    static QString mimeType();
    //! return all supported mime type
    static QStringList allMimeTypes();
    //! convert to mime data
    QMimeData *convertToMimeData();
    //! parse from mime data
    bool parseFromMimeData(const QMimeData *mimeData);
    //@}

protected:
    QString m_name,m_id;
    int m_freeBlocks;

    //! parse from urls
    bool parseFromUrls(const QList<QUrl> &urls);
};

#endif
