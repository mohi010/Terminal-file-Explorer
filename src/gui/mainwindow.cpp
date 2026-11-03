// ============================================================================
// File: src/gui/mainwindow.cpp
// Purpose: Implementations for the Qt GUI MainWindow.
// OS Concept: Direct bridge from GUI user interactions to C++17 core filesystem operations.
// ============================================================================

#include "mainwindow.hpp"
#include "../core/fs_ops.hpp"
#include "../core/search.hpp"
#include "../core/format.hpp"
#include <QSplitter>
#include <QHeaderView>
#include <QMessageBox>
#include <QInputDialog>
#include <QMenu>
#include <QKeySequence>
#include <QShortcut>
#include <QStatusBar>
#include <QStyle>
#include <QApplication>

namespace fx::gui {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("OS File Explorer (C++17 Core)"));
    resize(1050, 680);

    setupUi();
    setupToolbars();
    setupTree();
    setupTable();
    setupPropertiesDock();
    setupShortcuts();

    updateView();
}

void MainWindow::setupUi() {
    auto* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    auto* splitter = new QSplitter(Qt::Horizontal, centralWidget);
    m_treeWidget = new QTreeWidget(splitter);
    m_treeWidget->setHeaderLabel(QStringLiteral("Folders"));
    m_treeWidget->setMinimumWidth(220);

    m_tableModel = new FileTableModel(this);
    m_tableView = new QTableView(splitter);
    m_tableView->setModel(m_tableModel);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_tableView->horizontalHeader()->setStretchLastSection(true);
    m_tableView->horizontalHeader()->setSectionResizeMode(FileTableModel::ColName, QHeaderView::Stretch);
    m_tableView->setShowGrid(false);
    m_tableView->setAlternatingRowColors(true);

    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 3);

    auto* layout = new QHBoxLayout(centralWidget);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->addWidget(splitter);

    m_statusLabel = new QLabel(this);
    statusBar()->addWidget(m_statusLabel, 1);
}

void MainWindow::setupToolbars() {
    auto* navToolbar = addToolBar(QStringLiteral("Navigation"));
    navToolbar->setMovable(false);

    auto* actBack = navToolbar->addAction(style()->standardIcon(QStyle::SP_ArrowBack), QStringLiteral("Back"));
    auto* actFwd = navToolbar->addAction(style()->standardIcon(QStyle::SP_ArrowForward), QStringLiteral("Forward"));
    auto* actUp = navToolbar->addAction(style()->standardIcon(QStyle::SP_ArrowUp), QStringLiteral("Up"));
    auto* actRefresh = navToolbar->addAction(style()->standardIcon(QStyle::SP_BrowserReload), QStringLiteral("Refresh"));

    connect(actBack, &QAction::triggered, this, &MainWindow::onNavigateBack);
    connect(actFwd, &QAction::triggered, this, &MainWindow::onNavigateForward);
    connect(actUp, &QAction::triggered, this, &MainWindow::onNavigateUp);
    connect(actRefresh, &QAction::triggered, this, &MainWindow::onRefresh);

    m_addressBar = new QLineEdit(this);
    navToolbar->addWidget(m_addressBar);
    connect(m_addressBar, &QLineEdit::returnPressed, this, &MainWindow::onAddressEntered);

    auto* searchLabel = new QLabel(QStringLiteral(" Search: "), this);
    navToolbar->addWidget(searchLabel);
    m_searchBox = new QLineEdit(this);
    m_searchBox->setPlaceholderText(QStringLiteral("Find in folder..."));
    m_searchBox->setMaximumWidth(180);
    navToolbar->addWidget(m_searchBox);
    connect(m_searchBox, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
}

void MainWindow::setupTree() {
    connect(m_treeWidget, &QTreeWidget::itemExpanded, this, &MainWindow::onTreeItemExpanded);
    connect(m_treeWidget, &QTreeWidget::itemClicked, this, &MainWindow::onTreeItemClicked);

    // Root items: Drive roots on Windows or "/" on POSIX
#if defined(_WIN32)
    for (char drive = 'C'; drive <= 'Z'; ++drive) {
        std::string driveRoot = std::string(1, drive) + ":\\";
        std::error_code ec;
        if (std::filesystem::exists(driveRoot, ec)) {
            auto* item = new QTreeWidgetItem(m_treeWidget);
            item->setText(0, QString::fromStdString(driveRoot));
            item->setData(0, Qt::UserRole, QString::fromStdString(driveRoot));
            item->setIcon(0, style()->standardIcon(QStyle::SP_DriveHDIcon));
            // Add dummy child for lazy expansion
            new QTreeWidgetItem(item);
        }
    }
#else
    auto* rootItem = new QTreeWidgetItem(m_treeWidget);
    rootItem->setText(0, QStringLiteral("/"));
    rootItem->setData(0, Qt::UserRole, QStringLiteral("/"));
    rootItem->setIcon(0, style()->standardIcon(QStyle::SP_DriveHDIcon));
    new QTreeWidgetItem(rootItem);
#endif
}

void MainWindow::setupTable() {
    connect(m_tableView, &QTableView::doubleClicked, this, &MainWindow::onTableDoubleClicked);
    connect(m_tableView, &QTableView::customContextMenuRequested, this, &MainWindow::showContextMenu);
}

void MainWindow::setupPropertiesDock() {
    m_propertiesDock = new QDockWidget(QStringLiteral("Properties"), this);
    m_propertiesDock->setAllowedAreas(Qt::RightDockWidgetArea | Qt::LeftDockWidgetArea);

    m_propertiesText = new QTextEdit(m_propertiesDock);
    m_propertiesText->setReadOnly(true);
    m_propertiesDock->setWidget(m_propertiesText);

    addDockWidget(Qt::RightDockWidgetArea, m_propertiesDock);
}

void MainWindow::setupShortcuts() {
    new QShortcut(QKeySequence(Qt::Key_Backspace), this, SLOT(onNavigateUp()));
    new QShortcut(QKeySequence(Qt::ALT | Qt::Key_Left), this, SLOT(onNavigateBack()));
    new QShortcut(QKeySequence(Qt::ALT | Qt::Key_Right), this, SLOT(onNavigateForward()));
    new QShortcut(QKeySequence(Qt::Key_Delete), this, SLOT(onDelete()));
    new QShortcut(QKeySequence(Qt::Key_F2), this, SLOT(onRename()));
    new QShortcut(QKeySequence(Qt::Key_F5), this, SLOT(onRefresh()));
    new QShortcut(QKeySequence::Copy, this, SLOT(onCopy()));
    new QShortcut(QKeySequence::Cut, this, SLOT(onCut()));
    new QShortcut(QKeySequence::Paste, this, SLOT(onPaste()));
}

void MainWindow::updateView() {
    const auto& curPath = m_navigator.current();
    m_addressBar->setText(QString::fromStdString(curPath.u8string()));

    ListOptions opt;
    opt.showHidden = false;
    auto listRes = listDirectory(curPath, opt);
    if (!listRes.ok) {
        QMessageBox::warning(this, QStringLiteral("Error"), QString::fromStdString(listRes.error));
        return;
    }

    m_tableModel->setEntries(listRes.value);

    // Update status bar with entry counts and disk space info
    auto spRes = getSpace(curPath);
    QString statusMsg = QStringLiteral("%1 items").arg(listRes.value.size());
    if (spRes.ok) {
        statusMsg += QStringLiteral(" | Free Space: %1 of %2")
                         .arg(QString::fromStdString(formatSize(spRes.value.available)))
                         .arg(QString::fromStdString(formatSize(spRes.value.capacity)));
    }
    m_statusLabel->setText(statusMsg);
}

void MainWindow::populateTreeNode(QTreeWidgetItem* parentItem, const std::filesystem::path& dirPath) {
    parentItem->takeChildren(); // Remove dummy

    ListOptions opt;
    opt.showHidden = false;
    auto listRes = listDirectory(dirPath, opt);
    if (!listRes.ok) return;

    for (const auto& item : listRes.value) {
        if (item.type == EntryType::Directory) {
            auto* child = new QTreeWidgetItem(parentItem);
            child->setText(0, QString::fromUtf8(item.name.c_str()));
            child->setData(0, Qt::UserRole, QString::fromStdString(item.path.u8string()));
            child->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
            new QTreeWidgetItem(child); // Dummy for lazy expansion
        }
    }
}

void MainWindow::onTreeItemExpanded(QTreeWidgetItem* item) {
    QString pathStr = item->data(0, Qt::UserRole).toString();
    if (!pathStr.isEmpty()) {
        populateTreeNode(item, std::filesystem::u8path(pathStr.toStdString()));
    }
}

void MainWindow::onTreeItemClicked(QTreeWidgetItem* item, int) {
    QString pathStr = item->data(0, Qt::UserRole).toString();
    if (!pathStr.isEmpty()) {
        auto res = m_navigator.cd(pathStr.toStdString());
        if (res.ok) {
            updateView();
        }
    }
}

void MainWindow::onNavigateBack() {
    if (m_navigator.back().ok) updateView();
}

void MainWindow::onNavigateForward() {
    if (m_navigator.forward().ok) updateView();
}

void MainWindow::onNavigateUp() {
    if (m_navigator.up().ok) updateView();
}

void MainWindow::onRefresh() {
    updateView();
}

void MainWindow::onAddressEntered() {
    QString input = m_addressBar->text();
    auto res = m_navigator.cd(input.toStdString());
    if (!res.ok) {
        QMessageBox::warning(this, QStringLiteral("Navigation Error"), QString::fromStdString(res.error));
    }
    updateView();
}

void MainWindow::onTableDoubleClicked(const QModelIndex& index) {
    const auto* entry = m_tableModel->entryAt(index.row());
    if (!entry) return;

    if (entry->type == EntryType::Directory) {
        auto res = m_navigator.cd(entry->path.u8string());
        if (res.ok) {
            updateView();
        } else {
            QMessageBox::warning(this, QStringLiteral("Error"), QString::fromStdString(res.error));
        }
    } else {
        onShowProperties();
    }
}

void MainWindow::onSearchTextChanged(const QString& query) {
    if (query.trimmed().isEmpty()) {
        updateView();
        return;
    }

    SearchOptions opt;
    opt.maxDepth = 3;
    auto searchRes = searchByName(m_navigator.current(), query.toStdString(), opt);
    if (searchRes.ok) {
        m_tableModel->setEntries(searchRes.value);
        m_statusLabel->setText(QStringLiteral("%1 search result(s)").arg(searchRes.value.size()));
    }
}

void MainWindow::showContextMenu(const QPoint& pos) {
    QMenu menu(this);

    menu.addAction(QStringLiteral("New Folder"), this, &MainWindow::onNewFolder);
    menu.addAction(QStringLiteral("New File"), this, &MainWindow::onNewFile);
    menu.addSeparator();

    auto selected = m_tableView->selectionModel()->selectedRows();
    if (!selected.isEmpty()) {
        menu.addAction(QStringLiteral("Copy"), this, &MainWindow::onCopy);
        menu.addAction(QStringLiteral("Cut"), this, &MainWindow::onCut);
        menu.addAction(QStringLiteral("Rename"), this, &MainWindow::onRename);
        menu.addAction(QStringLiteral("Delete"), this, &MainWindow::onDelete);
        menu.addSeparator();
        menu.addAction(QStringLiteral("Properties"), this, &MainWindow::onShowProperties);
    }

    if (!m_clipboardPath.empty()) {
        menu.addAction(QStringLiteral("Paste"), this, &MainWindow::onPaste);
    }

    menu.exec(m_tableView->viewport()->mapToGlobal(pos));
}

void MainWindow::onNewFolder() {
    bool ok = false;
    QString name = QInputDialog::getText(this, QStringLiteral("New Folder"),
                                         QStringLiteral("Folder name:"), QLineEdit::Normal, QString(), &ok);
    if (ok && !name.trimmed().isEmpty()) {
        auto res = createDirectory(m_navigator.current() / name.toStdString());
        if (!res.ok) {
            QMessageBox::warning(this, QStringLiteral("Error"), QString::fromStdString(res.error));
        } else {
            updateView();
        }
    }
}

void MainWindow::onNewFile() {
    bool ok = false;
    QString name = QInputDialog::getText(this, QStringLiteral("New File"),
                                         QStringLiteral("File name:"), QLineEdit::Normal, QString(), &ok);
    if (ok && !name.trimmed().isEmpty()) {
        auto res = createFile(m_navigator.current() / name.toStdString());
        if (!res.ok) {
            QMessageBox::warning(this, QStringLiteral("Error"), QString::fromStdString(res.error));
        } else {
            updateView();
        }
    }
}

void MainWindow::onRename() {
    auto selected = m_tableView->selectionModel()->selectedRows();
    if (selected.isEmpty()) return;

    const auto* entry = m_tableModel->entryAt(selected.first().row());
    if (!entry) return;

    bool ok = false;
    QString newName = QInputDialog::getText(this, QStringLiteral("Rename"),
                                            QStringLiteral("New name:"), QLineEdit::Normal,
                                            QString::fromUtf8(entry->name.c_str()), &ok);
    if (ok && !newName.trimmed().isEmpty()) {
        auto res = renamePath(entry->path, newName.toStdString());
        if (!res.ok) {
            QMessageBox::warning(this, QStringLiteral("Rename Error"), QString::fromStdString(res.error));
        } else {
            updateView();
        }
    }
}

void MainWindow::onDelete() {
    auto selected = m_tableView->selectionModel()->selectedRows();
    if (selected.isEmpty()) return;

    const auto* entry = m_tableModel->entryAt(selected.first().row());
    if (!entry) return;

    auto reply = QMessageBox::question(this, QStringLiteral("Confirm Delete"),
                                       QStringLiteral("Are you sure you want to delete '%1'?")
                                           .arg(QString::fromUtf8(entry->name.c_str())),
                                       QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        auto res = removePath(entry->path, true);
        if (!res.ok) {
            QMessageBox::warning(this, QStringLiteral("Delete Error"), QString::fromStdString(res.error));
        } else {
            updateView();
        }
    }
}

void MainWindow::onCopy() {
    auto selected = m_tableView->selectionModel()->selectedRows();
    if (selected.isEmpty()) return;
    const auto* entry = m_tableModel->entryAt(selected.first().row());
    if (entry) {
        m_clipboardPath = entry->path;
        m_clipboardIsCut = false;
    }
}

void MainWindow::onCut() {
    auto selected = m_tableView->selectionModel()->selectedRows();
    if (selected.isEmpty()) return;
    const auto* entry = m_tableModel->entryAt(selected.first().row());
    if (entry) {
        m_clipboardPath = entry->path;
        m_clipboardIsCut = true;
    }
}

void MainWindow::onPaste() {
    if (m_clipboardPath.empty()) return;

    Result<bool> res;
    if (m_clipboardIsCut) {
        res = movePath(m_clipboardPath, m_navigator.current());
        if (res.ok) {
            m_clipboardPath.clear();
        }
    } else {
        res = copyPath(m_clipboardPath, m_navigator.current());
    }

    if (!res.ok) {
        QMessageBox::warning(this, QStringLiteral("Paste Error"), QString::fromStdString(res.error));
    } else {
        updateView();
    }
}

void MainWindow::onShowProperties() {
    auto selected = m_tableView->selectionModel()->selectedRows();
    if (selected.isEmpty()) return;

    const auto* entry = m_tableModel->entryAt(selected.first().row());
    if (!entry) return;

    auto infoRes = getInfo(entry->path);
    if (!infoRes.ok) return;

    const auto& info = infoRes.value;
    QString propHtml = QStringLiteral(
        "<h3>File Properties</h3>"
        "<p><b>Name:</b> %1</p>"
        "<p><b>Path:</b> %2</p>"
        "<p><b>Type:</b> %3</p>"
        "<p><b>Size:</b> %4 (%5 bytes)</p>"
        "<p><b>Modified:</b> %6</p>"
        "<p><b>Permissions:</b> %7</p>"
        "<p><b>Hidden:</b> %8</p>")
        .arg(QString::fromUtf8(info.name.c_str()))
        .arg(QString::fromStdString(info.path.u8string()))
        .arg(info.type == EntryType::Directory ? "Directory" : "File")
        .arg(QString::fromStdString(formatSize(info.size)))
        .arg(info.size)
        .arg(QString::fromStdString(formatTime(info.modified)))
        .arg(QString::fromStdString(formatPermissions(info.permissions)))
        .arg(info.isHidden ? "Yes" : "No");

    m_propertiesText->setHtml(propHtml);
    m_propertiesDock->show();
}

} // namespace fx::gui

