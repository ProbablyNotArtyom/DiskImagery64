#ifndef PETSCII_H
#define PETSCII_H

#include <QtCore/QtCore>

//! Tools for Petscii String Conversion
class Petscii {
public:
  enum AsciiConvertMode {
    ONLY_CASE,
    VALID_FILENAME,
    VALID_PATTERN
  };

  //! convert string for display with the CBM64.ttf font
  static QString convertToDisplayString(const QString &str);
  //! convert char for display with the CBM64.ttf font
  static QChar convertToDisplayChar(const QChar &c);
  
  //! convert petscii to ascii
  static QString convertToAscii(const QString &petscii,AsciiConvertMode mode);
  //! convert ascii to petscii
  static QString convertFromAscii(const QString &ascii);
};

#endif
