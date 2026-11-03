// ============================================================================
// File: src/gui/models.hpp
// Purpose: Custom QAbstractTableModel bridging core EntryInfo entries to QTableView.
// OS Concept: Separation of presentation model from OS directory entry data structures.
// ============================================================================

#pragma once

#include <QAbstractTableModel>
#include <vector>
#include "../core/types.hpp"

namespace fx::gui {

class FileTableModel : public QAbstractTableModel {
    Q_OBJECT

public:
    enum Columns {
        ColName = 0,
        ColType,
        ColSize,
        ColModified,
        ColPermissions,
        ColumnCount
    };

    explicit FileTableModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void setEntries(const std::vector<EntryInfo>& entries);
    const EntryInfo* entryAt(int row) const;

private:
    std::vector<EntryInfo> m_entries;
};

} // namespace fx::gui
