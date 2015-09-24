#include "treeForm.h"
#include <QTreeView>
#include "layerDragDropModel.h"
#include "../psdFileHdl.h"

TreeForm::TreeForm(QWidget *parent) :
    QWidget(parent)
{
    mTreeUi.setupUi(this);
    mLayout = new QVBoxLayout;
}

TreeForm::~TreeForm()
{
    if(NULL == mTreeView)
    {
          delete mTreeView;
          mTreeView = NULL;
    }
    if(NULL == mLayout)
    {
        delete mLayout;
        mLayout = NULL;
    }
}

void
TreeForm::BindItems(QStringList& p_rItems)
{

    /*
    p_rItems << tr("Widgets")
             << tr("  QWidget")
             << tr("    QWidget")
             << tr("  QDialog")
             << tr("Tools")
             << tr("  Qt Designer")
             << tr("  Qt Assistant");
    */
    layerDragDropModel *model = new layerDragDropModel(p_rItems, this);
    QModelIndex mIndex = model->index(0, 0, QModelIndex());
    mTreeView = new QTreeView;
    //mTreeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
   // mTreeView->setDragEnabled(true);
    //mTreeView->setAcceptDrops(true);
    //mTreeView->setDropIndicatorShown(true);
    mTreeView->expand(mIndex);
    mTreeView->scrollTo(mIndex);
    //mTreeView->header()->setResizeMode(QHeaderView::Stretch);
    mTreeView->setModel(model);

    mLayout->addWidget(mTreeView);
    setLayout(mLayout);
    resize(640, 480);
}

