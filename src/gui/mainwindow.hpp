// ============================================================================
// File: src/gui/mainwindow.hpp
// Purpose: Qt GUI Main Window interface for OS File Explorer.
// OS Concept: Graphical user interface shell abstraction over OS filesystem core.
// ============================================================================

#pragma once

#include <QMainWindow>
#include <QTreeView>
#include <QTableView>
#include <QTreeWidget>
#include <QLineEdit>
#include <QLabel>
#include <QToolBar>
#include <QDockWidget>
#include <QTextEdit>
#include "../core/navigator.hpp"
#include "models.hpp"

namespace fx::gui {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

private slots:
    void onNavigateBack();
    void onNavigateForward();
    void onNavigateUp();
    void onRefresh();
    void onAddressEntered();
    void onTableDoubleClicked(const QModelIndex& index);
    void onTreeItemExpanded(QTreeWidgetItem* item);
    void onTreeItemClicked(QTreeWidgetItem* item, int column);
    void onSearchTextChanged(const QString& query);
    void showContextMenu(const QPoint& pos);
    void onNewFolder();
    void onNewFile();
    void onRename();
    void onDelete();
    void onCopy();
    void onCut();
    void onPaste();
    void onShowProperties();

private:
    void setupUi();
    void setupToolbars();
    void setupTree();
    void setupTable();
    void setupPropertiesDock();
    void setupShortcuts();
    void updateView();
    void populateTreeNode(QTreeWidgetItem* parentItem, const std::filesystem::path& dirPath);

    Navigator m_navigator;
    FileTableModel* m_tableModel = nullptr;
    QTableView* m_tableView = nullptr;
    QTreeWidget* m_treeWidget = nullptr;
    QLineEdit* m_addressBar = nullptr;
    QLineEdit* m_searchBox = nullptr;
    QLabel* m_statusLabel = nullptr;
    QDockWidget* m_propertiesDock = nullptr;
    QTextEdit* m_propertiesText = nullptr;

    // Clipboard state for GUI Cut/Copy/Paste operations
    std::filesystem::path m_clipboardPath;
    bool m_clipboardIsCut = false;
};

} // namespace fx::gui
