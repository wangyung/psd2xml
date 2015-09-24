#ifndef LAYER_TREE_ITEM_H
#define LAYER_TREE_ITEM_H

#include <QList>
#include <QVariant>


class layerTreeItem
{
public:
    layerTreeItem(const QList<QVariant> &data, layerTreeItem *parent = 0);
    ~layerTreeItem();

    void appendChild(layerTreeItem *child);

    layerTreeItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    bool insertChild(int row, layerTreeItem *item);
    layerTreeItem *parent();
    bool removeChild(int row);
    int row() const;
    bool setData(int column, const QVariant &data);

private:
    QList<layerTreeItem*> mChildItems;
    QList<QVariant> mItemData;
    layerTreeItem *mParentItem;
};

#endif
