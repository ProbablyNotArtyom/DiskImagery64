#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QtGui/QtGui>
#include "nethost.h"

#include <QDialog>
#include <QLineEdit>
#include <QListView>
#include <QCheckBox>

#include "ColorPickerButton.h"

class Preferences : public QDialog
{
    Q_OBJECT

public:
    Preferences(QWidget *parent=0);
    ~Preferences();

    void load();
    void save();

    // ----- Defaults -----
    // image defaults
    static void getImageDefaults(QString &file,
                                 QString &title,
                                 QString &id,
                                 int &counter);
    static void setImageDefaults(const QString &file,
                                 const QString &title,
                                 const QString &id,
                                 int counter);
    static void getNextImageName(QString &file,QString &title,QString &id);
    // emulator defaults
    static void getEmulatorDefaults(QString &app,
                                    QString &mountArgs,
                                    QString &runArgs);
    static void setEmulatorDefaults(const QString &app,
                                    const QString &mountArgs,
                                    const QString &runArgs);
    // font defaults
    static void setFontDefaults(bool shifted,const QFont &font,const QFont &shiftedFont);
    static void getFontDefaults(bool &shifted,QFont &font,QFont &shiftedFont);
    static void setColorDefaults(const QColor &fgColor, const QColor &bgColor);
    static void getColorDefaults(QColor &fgColor, QColor &bgColor);
    // separator defaults
    static void setSeparatorDefaults(const QStringList &templates);
    static void getSeparatorDefaults(QStringList &templates);
    // network defaults
    static void getNetworkDefaults(AddrPair &addrPair);
    static void setNetworkDefaults(const AddrPair &addrPair);
    static void getWarpCopyDefaults(QString &prg,bool &patch);
    static void setWarpCopyDefaults(const QString &prg,bool patch);

protected slots:
    void onAddSeparator();
    void onDelSeparator();
    void onPickEmuApp();
    void onPickFont();
    void onPickShiftedFont();
    void onFontColourFG(QColor color);
    void onFontColourBG(QColor color);
    void onPickWcPrg();

protected:
    // image controls
    QLineEdit *m_imgFileEdit;
    QLineEdit *m_imgTitleEdit;
    QLineEdit *m_imgIdEdit;
    QLineEdit *m_imgCounterEdit;
    // font controls
    QCheckBox *m_fontShiftedCheck;
    QLineEdit *m_fontNameEdit;
    QLineEdit *m_fontSizeEdit;
    ColorPickerButton *m_fontColourFGButton;
    ColorPickerButton *m_fontColourBGButton;
    QLineEdit *m_fontShiftedNameEdit;
    QLineEdit *m_fontShiftedSizeEdit;
    // separator controls
    QListView *m_separatorView;
    QStringListModel *m_separatorModel;
    // emulator controls
    QLineEdit *m_emuAppEdit;
    QLineEdit *m_emuMountArgsEdit;
    QLineEdit *m_emuRunArgsEdit;
    // network controls
    QLineEdit *m_netMyAddrEdit;
    QLineEdit *m_netC64AddrEdit;
    QLineEdit *m_netWarpCopyPrgEdit;
    QCheckBox *m_netWarpCopyPatchCheck;

    QWidget *createImageGroup(QWidget *parent);
    QWidget *createFontGroup(QWidget *parent);
    QWidget *createSeparatorGroup(QWidget *parent);
    QWidget *createEmulatorGroup(QWidget *parent);
    QWidget *createNetworkGroup(QWidget *parent);

    QFont currentFont();

    static bool insertCounter(const QString &pattern,int counter,QString &result);
};

#endif
