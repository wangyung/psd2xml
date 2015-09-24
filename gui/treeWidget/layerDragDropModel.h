#ifndef TREE_DRAGDROP_MODEL_H
#define TREE_DRAGDROP_MODEL_H

#include "layerTreeModel.h"

class layerDragDropModel : public layerTreeModel
{
    Q_OBJECT

public:
    layerDragDropModel(const QStringList &strings, QObject *parent = 0);

    Qt::ItemFlags flags(const QModelIndex &index) const;

    bool dropMimeData(const QMimeData *data, Qt::DropAction action,
                      int row, int column, const QModelIndex &parent);
    QMimeData *mimeData(const QModelIndexList &indexes) const;
    QStringList mimeTypes() const;
    Qt::DropActions supportedDropActions() const;
};

#endif
