#include "petscii.h"

QString Petscii::convertToDisplayString(const QString &str) {
    QString result;
    int size = str.size();
    result.resize(str.size());
    for (int i = 0; i < size; i++)
        result[i] = convertToDisplayChar(str[i]);
    return result;
}

QChar Petscii::convertToDisplayChar(const QChar &c) {
    short code = short(c.unicode());
    // shift latin1 special chars
    // non breakin space
    if (code == 0xa0) code = 0x20;
    // soft hyphen
    if (code == 0xad) code = 0xed;
    return code;
}

QString Petscii::convertToAscii(const QString &petscii, AsciiConvertMode mode) {
    QString ascii;
    int size = petscii.size();
    for (int i = 0; i < size; i++) {
        QChar c = petscii[i];
        if ((c >= 'A') && (c <= 'Z')) {
            c = QChar(c.unicode() - 'A' + 'a');
        } else if ((c >= 'a') && (c <= 'z')) {
            c = QChar(c.unicode() - 'a' + 'A');
        } else if (mode == VALID_FILENAME) {
            if ((c == '/') || (c == ':') || (c == '\\')) {
                c = '-';
            }
        } else if (mode == VALID_PATTERN) {
            if (c.unicode() < 0x20 || c.unicode() > 0x5f)
                c = '?';
        }
        ascii[i] = c;
    }
    return ascii;
}

QString Petscii::convertFromAscii(const QString &ascii) {
    // same operation here
    return convertToAscii(ascii, ONLY_CASE);
}
