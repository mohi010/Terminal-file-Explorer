// ============================================================================
// File: src/gui/mainwindow.hpp
// Purpose: Qt GUI Main Window interface for macOS-styled OS File Explorer.
// OS Concept: Graphical user interface shell abstraction over OS filesystem core.
// ============================================================================

#pragma once

#include <QMainWindow>
#include <QTreeView>
#include <QTableView>
#include <QListView>
#include <QStackedWidget>
#include <QTreeWidget>
#include <QLineEdit>
#include <QLabel>
#include <QToolBar>
#include <QDockWidget>
#include <QTextEdit>
#include <QToolButton>
#include "../core/navigator.hpp"
#include "models.hpp"
#include "mac_theme.hpp"

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
    void onEntryDoubleClicked(const QModelIndex& index);
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
    void onToggleTheme();
    void onSetViewModeIcons();
    void onSetViewModeList();
    void onSetViewModeCompact();

private:
    void setupHeader();
    void setupUi();
    void setupToolbars();
    void setupTree();
    void setupViews();
    void setupPropertiesDock();
    void setupShortcuts();
    void updateView();
    void populateTreeNode(QTreeWidgetItem* parentItem, const std::filesystem::path& dirPath);
    void applyTheme();
    const EntryInfo* getSelectedEntry() const;
    QString getDriveDisplayName(char drive);

    Navigator m_navigator;
    FileTableModel* m_tableModel = nullptr;

    // Multi-View Stack (macOS Icons Grid, List Table, and Compact Views)
    QStackedWidget* m_viewStack = nullptr;
    QListView* m_iconView = nullptr;
    QTableView* m_tableView = nullptr;
    QListView* m_compactView = nullptr;

    QTreeWidget* m_treeWidget = nullptr;
    QLineEdit* m_addressBar = nullptr;
    QLineEdit* m_searchBox = nullptr;
    QLabel* m_statusLabel = nullptr;
    QLabel* m_pathBreadcrumb = nullptr;
    QLabel* m_titleLabel = nullptr;
    QDockWidget* m_propertiesDock = nullptr;
    QTextEdit* m_propertiesText = nullptr;
    QToolButton* m_themeBtn = nullptr;

    QAction* m_actViewIcons = nullptr;
    QAction* m_actViewList = nullptr;
    QAction* m_actViewCompact = nullptr;

    bool m_darkMode = true;
    int m_currentViewMode = 0; // 0 = Icons, 1 = List, 2 = Compact

    // Clipboard state for GUI Cut/Copy/Paste operations
    std::filesystem::path m_clipboardPath;
    bool m_clipboardIsCut = false;
};

} // namespace fx::gui
