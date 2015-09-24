#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <string>
#include <QDockWidget>
#include "treeWidget/treeForm.h"
#include <QStringList>
#include "psdFileHdl.h"
#include "imageWidget/imageWidget.h"
#include <QtDebug>
#include <QSplitter>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    mUi = new Ui::MainWindow;
    setWindowTitle(tr("psd2xml tool"));
    setAcceptDrops(true);
    mUi->setupUi(this);
    initConnect();
}

MainWindow::~MainWindow()
{

    if(NULL != mUi)
    {
        delete mUi;
        mUi=NULL;
    }

}

void
MainWindow::initConnect()
{
    connect(mUi->actionOpen,
            SIGNAL(triggered()),
            this,
            SLOT(doOpen()));
}

void
MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls())
    {
        QString localFile;
        QRegExp rx("\\.(psd)$", Qt::CaseInsensitive);
        foreach(QUrl url , event->mimeData()->urls())
        {
            localFile = url.toLocalFile();
            if(rx.indexIn(localFile) >= 0)
            {
                event->accept();
                return;
            }
            else
                event->ignore();
        }
    }
    else
        event->ignore();
}

void
MainWindow::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasUrls())
    {
        QString localFile;
        QRegExp rx("\\.(psd)$", Qt::CaseInsensitive);
        foreach(QUrl url , event->mimeData()->urls())
        {
            localFile = url.toLocalFile();
            if(rx.indexIn(localFile) >= 0)
            {
                event->accept();
                openFile(localFile);
                return;
            }
            else
                event->ignore();
        }
    }
    else
        event->ignore();
}

void
MainWindow::openFile(QString &filePath)
{
    InstancePsd(filePath);
}

void
MainWindow::doOpen()
{
    QString rQstrFileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                    "",
                                                    tr("Images (*.psd)"));
    openFile(rQstrFileName);

}

// ----------------- PsdProcessWnd -----------------
PsdProcessWnd::PsdProcessWnd(QWidget *parent) :
    MainWindow(parent)
{
    mTreeDockWidget=NULL;
    mPsdHdl=NULL;
    mTreeForm=NULL;
    mCanvasDockWidget=NULL;
    iniDockWidget();
}

PsdProcessWnd::~PsdProcessWnd()
{
    if(NULL != mPsdHdl)
    {
        delete mPsdHdl;
        mPsdHdl=NULL;
    }
    if(NULL != mTreeForm)
    {
        delete mTreeForm;
        mTreeForm=NULL;
    }
    if(NULL != mTreeDockWidget)
    {
        delete mTreeDockWidget;
        mTreeDockWidget=NULL;
    }
}

void
PsdProcessWnd::InstancePsd(QString& filePath)
{
    string rFileNameStr = filePath.toStdString();
    if(NULL == mPsdHdl)
        mPsdHdl = new psdHandle;
    else
    {
        QMessageBox::warning(this,
                             tr("Warning"),
                             tr("Multiple psd does NOT support"),
                             QMessageBox::Yes);
        return;
    }

    if(NULL == mPsdHdl)
        return;

    if(NULL == mTreeForm)
         mTreeForm = new TreeForm();
    if(NULL == mTreeForm)
        return;



    mPsdHdl->CreateHandler(rFileNameStr);
    QStringList layerItems,imgItems;
    mPsdHdl->PrepareItem(layerItems,imgItems);
    mTreeForm->BindItems(layerItems);


    QStringListIterator javaStyleIterator(imgItems);
    while (javaStyleIterator.hasNext())
        mImageWidget->setPixmap(javaStyleIterator.next().toLocal8Bit().constData());

    //tabifyDockWidget(mTreeDockWidget,mCanvasDockWidget);
     //splitDockWidget(mTreeDockWidget,mCanvasDockWidget,Qt::Horizontal);
    mTreeDockWidget->setVisible(true);
    mCanvasDockWidget->setVisible(true);
}


void
PsdProcessWnd::iniDockWidget()
{

    mTreeForm = new TreeForm();
    mTreeDockWidget = new QDockWidget("Tree",this);
    mTreeDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea);
    mTreeDockWidget->setFeatures(QDockWidget::AllDockWidgetFeatures);
    mTreeDockWidget->setFloating(false);
    mTreeDockWidget->setWidget(mTreeForm);
    mTreeDockWidget->setVisible(false);

    addDockWidget(Qt::LeftDockWidgetArea,mTreeDockWidget);

    mImageWidget = new ImageWidget(this);
    mCanvasDockWidget = new QDockWidget("Image",this);
    mCanvasDockWidget->setAllowedAreas(Qt::RightDockWidgetArea);
    mCanvasDockWidget->setFeatures(QDockWidget::AllDockWidgetFeatures);
    mCanvasDockWidget->setFloating(false);
    mCanvasDockWidget->setWidget(mImageWidget);
    mCanvasDockWidget->setVisible(false);
    addDockWidget(Qt::RightDockWidgetArea,mCanvasDockWidget);
}
