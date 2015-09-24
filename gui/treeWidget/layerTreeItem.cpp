#include <QStringList>
#include "layerTreeItem.h"

layerTreeItem::layerTreeItem(const QList<QVariant> &data, layerTreeItem *parent)
{
    mParentItem = parent;
    mItemData = data;
}

layerTreeItem::~layerTreeItem()
{
    qDeleteAll(mChildItems);
}

void layerTreeItem::appendChild(layerTreeItem *item)
{
    mChildItems.append(item);
}

layerTreeItem *layerTreeItem::child(int row)
{
    return mChildItems.value(row);
}

int layerTreeItem::childCount() const
{
    return mChildItems.count();
}

int layerTreeItem::columnCount() const
{
    return mItemData.count();
}

QVariant layerTreeItem::data(int column) const
{
    return mItemData.value(column);
}

bool layerTreeItem::insertChild(int row, layerTreeItem *item)
{
    if (row < 0 || row > mChildItems.count())
        return false;

    mChildItems.insert(row, item);
    return true;
}

layerTreeItem *layerTreeItem::parent()
{
    return mParentItem;
}

bool layerTreeItem::removeChild(int row)
{
    if (row < 0 || row >= mChildItems.count())
        return false;

    delete mChildItems.takeAt(row);
    return true;
}

int layerTreeItem::row() const
{
    if (mParentItem)
        return mParentItem->mChildItems.indexOf(const_cast<layerTreeItem*>(this));

    return 0;
}

bool layerTreeItem::setData(int column, const QVariant &data)
{
    if (column < 0 || column >= mItemData.count())
        return false;

    mItemData.replace(column, data);
    return true;
}
