// ============================================================================
// File: src/gui/models.cpp
// Purpose: Implementations for the QAbstractTableModel.
// OS Concept: Presentation layer table mapping.
// ============================================================================

#include "models.hpp"
#include "mac_theme.hpp"
#include "../core/format.hpp"
#include <QIcon>
#include <QApplication>
#include <QStyle>

namespace fx::gui {

FileTableModel::FileTableModel(QObject* parent)
    : QAbstractTableModel(parent) {}

int FileTableModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(m_entries.size());
}

int FileTableModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return ColumnCount;
}

QVariant FileTableModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= static_cast<int>(m_entries.size())) {
        return QVariant();
    }

    const auto& entry = m_entries[static_cast<size_t>(index.row())];

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case ColName:
                return QString::fromUtf8(entry.name.c_str());
            case ColType:
                if (entry.type == EntryType::Directory) return QStringLiteral("Folder");
                if (entry.type == EntryType::Symlink) return QStringLiteral("Symbolic Link");
                return QStringLiteral("File");
            case ColSize:
                return entry.type == EntryType::Directory ? QStringLiteral("-") :
                       QString::fromStdString(formatSize(entry.size));
            case ColModified:
                return QString::fromStdString(formatTime(entry.modified));
            case ColPermissions:
                return QString::fromStdString(formatPermissions(entry.permissions));
            default:
                break;
        }
    } else if (role == Qt::DecorationRole && index.column() == ColName) {
        if (entry.type == EntryType::Directory) {
            return MacTheme::folderIcon(96);
        }
        return MacTheme::fileIcon(QString::fromStdString(entry.path.extension().string()), 96);
    }

    return QVariant();
}

QVariant FileTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case ColName: return QStringLiteral("Name");
            case ColType: return QStringLiteral("Type");
            case ColSize: return QStringLiteral("Size");
            case ColModified: return QStringLiteral("Modified");
            case ColPermissions: return QStringLiteral("Permissions");
            default: break;
        }
    }
    return QVariant();
}

void FileTableModel::setEntries(const std::vector<EntryInfo>& entries) {
    beginResetModel();
    m_entries = entries;
    endResetModel();
}

const EntryInfo* FileTableModel::entryAt(int row) const {
    if (row >= 0 && row < static_cast<int>(m_entries.size())) {
        return &m_entries[static_cast<size_t>(row)];
    }
    return nullptr;
}

} // namespace fx::gui

