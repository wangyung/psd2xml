#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtGui/QMdiArea>
#include <QtCore/QList>

class TreeForm;
class psdHandle;
class QMimeData;
class QDockWidget;
class ImageWidget;

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QString mainWindowTitle();
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

protected:
    virtual void InstancePsd(QString& filePath) = 0;

private:
    void initConnect();
    void openFile(QString& filePath);
    Ui::MainWindow *mUi;


private slots:
    void doOpen();
};

class PsdProcessWnd : public MainWindow
{
    Q_OBJECT

public:
    explicit PsdProcessWnd(QWidget *parent = 0);
    ~PsdProcessWnd();
    virtual void InstancePsd(QString& filePath);

private:
    void iniDockWidget();

    QDockWidget* mTreeDockWidget;
    QDockWidget* mCanvasDockWidget;
    psdHandle* mPsdHdl;
    TreeForm* mTreeForm;
    ImageWidget* mImageWidget;
};

#endif // MAINWINDOW_H
