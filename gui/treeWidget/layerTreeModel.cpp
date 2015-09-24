#include <QtGui>
#include "layerTreeItem.h"
#include "layerTreeModel.h"

layerTreeModel::layerTreeModel(const QStringList &strings, QObject *parent)
    : QAbstractItemModel(parent)
{
    QList<QVariant> rootData;
    rootData << "GroupLayer";
    mRootItem = new layerTreeItem(rootData);
    setupModelData(strings, mRootItem);
}

layerTreeModel::~layerTreeModel()
{
    delete mRootItem;
}

int layerTreeModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<layerTreeItem*>(parent.internalPointer())->columnCount();
    else
        return mRootItem->columnCount();
}

QVariant layerTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    layerTreeItem *item = static_cast<layerTreeItem*>(index.internalPointer());

    return item->data(index.column());
}

Qt::ItemFlags layerTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant layerTreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return mRootItem->data(section);

    return QVariant();
}

QModelIndex layerTreeModel::index(int row, int column, const QModelIndex &parent)
            const
{
    layerTreeItem *parentItem;

    if (!parent.isValid())
        parentItem = mRootItem;
    else
        parentItem = static_cast<layerTreeItem*>(parent.internalPointer());

    layerTreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

bool layerTreeModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    layerTreeItem *parentItem;

    if (!parent.isValid())
        parentItem = mRootItem;
    else
        parentItem = static_cast<layerTreeItem*>(parent.internalPointer());

    if (position < 0 || position > parentItem->childCount())
        return false;

    QList<QVariant> blankList;
    for (int column = 0; column < columnCount(); ++column)
        blankList << QVariant("");

    beginInsertRows(parent, position, position + rows - 1);

    for (int row = 0; row < rows; ++row) {
        layerTreeItem *newItem = new layerTreeItem(blankList, parentItem);
        if (!parentItem->insertChild(position, newItem))
            break;
    }

    endInsertRows();
    return true;
}

QModelIndex layerTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    layerTreeItem *childItem = static_cast<layerTreeItem*>(index.internalPointer());
    layerTreeItem *parentItem = childItem->parent();

    if (parentItem == mRootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

bool layerTreeModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    layerTreeItem *parentItem;

    if (!parent.isValid())
        parentItem = mRootItem;
    else
        parentItem = static_cast<layerTreeItem*>(parent.internalPointer());

    if (position < 0 || position > parentItem->childCount())
        return false;

    beginRemoveRows(parent, position, position + rows - 1);

    for (int row = 0; row < rows; ++row) {
        if (!parentItem->removeChild(position))
            break;
    }

    endRemoveRows();
    return true;
}

int layerTreeModel::rowCount(const QModelIndex &parent) const
{
    layerTreeItem *parentItem;

    if (!parent.isValid())
        parentItem = mRootItem;
    else
        parentItem = static_cast<layerTreeItem*>(parent.internalPointer());

    return parentItem->childCount();
}

bool layerTreeModel::setData(const QModelIndex &index,
                        const QVariant &value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    layerTreeItem *item = static_cast<layerTreeItem*>(index.internalPointer());

    if (item->setData(index.column(), value))
        emit dataChanged(index, index);
    else
        return false;

    return true;
}

void layerTreeModel::setupModelData(const QStringList &lines, layerTreeItem *parent)
{
    QList<layerTreeItem*> parents;
    QList<int> indentations;
    parents << parent;
    indentations << 0;

    int number = 0;

    while (number < lines.count()) {
        int position = 0;
        while (position < lines[number].length()) {
            if (lines[number].mid(position, 1) != " ")
                break;
            position++;
        }

        QString lineData = lines[number].mid(position).trimmed();

        if (!lineData.isEmpty()) {
            // Read the column data from the rest of the line.
            QStringList columnStrings = lineData.split("\t", QString::SkipEmptyParts);
            QList<QVariant> columnData;
            for (int column = 0; column < columnStrings.count(); ++column)
                columnData << columnStrings[column];

            if (position > indentations.last()) {
                // The last child of the current parent is now the new parent
                // unless the current parent has no children.

                if (parents.last()->childCount() > 0) {
                    parents << parents.last()->child(parents.last()->childCount()-1);
                    indentations << position;
                }
            } else {
                while (position < indentations.last() && parents.count() > 0) {
                    parents.pop_back();
                    indentations.pop_back();
                }
            }

            // Append a new item to the current parent's list of children.
            parents.last()->appendChild(new layerTreeItem(columnData, parents.last()));
        }

        number++;
    }
}
