#include <QtGui>
#include "layerDragDropModel.h"

layerDragDropModel::layerDragDropModel(const QStringList &strings, QObject *parent)
    : layerTreeModel(strings, parent)
{
}

bool layerDragDropModel::dropMimeData(const QMimeData *data,
    Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    if (action == Qt::IgnoreAction)
        return true;

    if (!data->hasFormat("text/plain"))
        return false;

    int beginRow;

    if (row != -1)
        beginRow = row;
    else if (parent.isValid())
        beginRow = 0;
    else
        beginRow = rowCount(QModelIndex());

    QByteArray encodedData = data->data("text/plain");
    QDataStream stream(&encodedData, QIODevice::ReadOnly);
    QHash<qint64, QMap<int,QHash<int,QString> > > newItems;

    while (!stream.atEnd()) {
        qint64 id;
        int row;
        int column;
        QString text;
        stream >> id >> row >> column >> text;
        newItems[id][row][column] = text;
    }
    int rows = newItems.count();

    insertRows(beginRow, rows, parent);
    QMap<int,QHash<int,QString> > childItems;
    foreach (childItems, newItems.values()) {
        QHash<int,QString> rowItems;
        foreach (rowItems, childItems.values()) {
            foreach (int column, rowItems.keys()) {
                QModelIndex idx = index(beginRow, column, parent);
                setData(idx, rowItems[column]);
            }
            ++beginRow;
        }
    }

    return true;
}

Qt::ItemFlags layerDragDropModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = layerTreeModel::flags(index);

    if (index.isValid())
        return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
    else
        return Qt::ItemIsDropEnabled | defaultFlags;
}

QMimeData *layerDragDropModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    foreach (QModelIndex index, indexes) {
        if (index.isValid()) {
            QString text = data(index, Qt::DisplayRole).toString();
            stream << index.internalId() << index.row() << index.column() << text;
        }
    }

    mimeData->setData("text/plain", encodedData);
    return mimeData;
}

QStringList layerDragDropModel::mimeTypes() const
{
    QStringList types;
    types << "text/plain";
    return types;
}

Qt::DropActions layerDragDropModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}
