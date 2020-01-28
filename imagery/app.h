#ifndef App_H
#define App_H

#include <QtGui/QtGui>
#include <QApplication>

class App : public QApplication
{
    Q_OBJECT

public:
    App(int &argc,char **argv);
    ~App();

    void openFile(const QString &fileName);

    static int getNumFileWin();
    static int getNumDImageWin();

protected:
    bool event(QEvent *);
    void handleFileOpenEvent(QFileOpenEvent *);
};

#endif
