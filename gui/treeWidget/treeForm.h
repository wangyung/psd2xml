#ifndef TREE_FORM_H
#define TREE_FORM_H

#include "ui_treeWidget.h"
#include <QtGui>

class TreeForm : public QWidget
{
    Q_OBJECT

public:
    explicit TreeForm(QWidget *parent = 0);
    ~TreeForm();
    void BindItems(QStringList& p_rItems);


private:
    QModelIndex mIndex;
    QVBoxLayout *mLayout;
    QTreeView *mTreeView;

private:
    Ui_TreeFormWidget mTreeUi;

};

#endif // TREE_FORM_H
