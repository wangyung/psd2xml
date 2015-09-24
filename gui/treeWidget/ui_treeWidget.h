/********************************************************************************
** Form generated from reading UI file 'treeWidget.ui'
**
** Created: Fri Nov 11 11:56:16 2011
**      by: Qt User Interface Compiler version 4.6.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TREEWIDGET_H
#define UI_TREEWIDGET_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_TreeFormWidget
{
public:

    void setupUi(QWidget *TreeFormWidget)
    {
        if (TreeFormWidget->objectName().isEmpty())
            TreeFormWidget->setObjectName(QString::fromUtf8("TreeFormWidget"));
        TreeFormWidget->resize(156, 439);

        retranslateUi(TreeFormWidget);

        QMetaObject::connectSlotsByName(TreeFormWidget);
    } // setupUi

    void retranslateUi(QWidget *TreeFormWidget)
    {
        TreeFormWidget->setWindowTitle(QApplication::translate("TreeFormWidget", "Form", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class TreeFormWidget: public Ui_TreeFormWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TREEWIDGET_H
