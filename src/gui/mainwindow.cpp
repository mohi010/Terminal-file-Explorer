// ============================================================================
// File: src/gui/mainwindow.cpp
// Purpose: Implementations for macOS-styled Qt GUI MainWindow with multi-view modes
//          (Icons grid, List details, Columns compact) and Windows drive volume names.
// ============================================================================

#include "mainwindow.hpp"
#include "mac_theme.hpp"
#include "../core/fs_ops.hpp"
#include "../core/search.hpp"
#include "../core/format.hpp"
#include <QSplitter>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QInputDialog>
#include <QMenu>
#include <QKeySequence>
#include <QShortcut>
#include <QStatusBar>
#include <QStyle>
#include <QApplication>
#include <QToolButton>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace fx::gui {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("Finder — OS File Explorer"));
    resize(1140, 720);

    setupUi();
    setupToolbars();
    setupTree();
    setupViews();
    setupPropertiesDock();
    setupShortcuts();

    applyTheme();
    updateView();
}

QString MainWindow::getDriveDisplayName(char drive) {
#if defined(_WIN32)
    std::wstring wDriveRoot = std::wstring(1, static_cast<wchar_t>(drive)) + L":\\";
    wchar_t volumeName[MAX_PATH + 1] = {0};
    if (GetVolumeInformationW(wDriveRoot.c_str(), volumeName, MAX_PATH, nullptr, nullptr, nullptr, nullptr, 0) && volumeName[0] != L'\0') {
        return QStringLiteral("%1 (%2:)").arg(QString::fromWCharArray(volumeName)).arg(drive);
    }
    return QStringLiteral("Local Disk (%1:)").arg(drive);
#else
    return QStringLiteral("Macintosh HD (/)");
#endif
}

void MainWindow::applyTheme() {
    setStyleSheet(MacTheme::getMacStylesheet(m_darkMode));
    if (m_themeBtn) {
        m_themeBtn->setText(m_darkMode ? QStringLiteral("☀️ Light") : QStringLiteral("🌙 Dark"));
    }
}

void MainWindow::onToggleTheme() {
    m_darkMode = !m_darkMode;
    applyTheme();
}

void MainWindow::setupHeader() {
    // macOS Unified Top Header
    auto* headerWidget = new QWidget(this);
    headerWidget->setObjectName(QStringLiteral("MacHeader"));
    headerWidget->setFixedHeight(40);

    auto* headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(12, 0, 12, 0);
    headerLayout->setSpacing(8);

    // 1. Traffic Light Buttons
    auto* btnClose = new QToolButton(headerWidget);
    btnClose->setObjectName(QStringLiteral("TrafficClose"));
    btnClose->setIcon(MacTheme::closeButtonIcon(12));
    btnClose->setToolTip(QStringLiteral("Close"));
    connect(btnClose, &QToolButton::clicked, this, &MainWindow::close);

    auto* btnMin = new QToolButton(headerWidget);
    btnMin->setObjectName(QStringLiteral("TrafficMin"));
    btnMin->setIcon(MacTheme::minimizeButtonIcon(12));
    btnMin->setToolTip(QStringLiteral("Minimize"));
    connect(btnMin, &QToolButton::clicked, this, &MainWindow::showMinimized);

    auto* btnMax = new QToolButton(headerWidget);
    btnMax->setObjectName(QStringLiteral("TrafficMax"));
    btnMax->setIcon(MacTheme::maximizeButtonIcon(12));
    btnMax->setToolTip(QStringLiteral("Zoom / Maximize"));
    connect(btnMax, &QToolButton::clicked, this, [this]() {
        if (isMaximized()) showNormal();
        else showMaximized();
    });

    headerLayout->addWidget(btnClose);
    headerLayout->addWidget(btnMin);
    headerLayout->addWidget(btnMax);
    headerLayout->addSpacing(16);

    // 2. Centered Window Title
    m_titleLabel = new QLabel(QStringLiteral("OS File Explorer"), headerWidget);
    m_titleLabel->setObjectName(QStringLiteral("MacTitleLabel"));
    headerLayout->addWidget(m_titleLabel);

    headerLayout->addStretch();

    // 3. Dark/Light Theme Toggle
    m_themeBtn = new QToolButton(headerWidget);
    m_themeBtn->setText(m_darkMode ? QStringLiteral("☀️ Light") : QStringLiteral("🌙 Dark"));
    m_themeBtn->setToolTip(QStringLiteral("Toggle macOS Dark / Light Theme"));
    connect(m_themeBtn, &QToolButton::clicked, this, &MainWindow::onToggleTheme);
    headerLayout->addWidget(m_themeBtn);

    setMenuWidget(headerWidget);
}

void MainWindow::setupUi() {
    setupHeader();

    auto* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    auto* splitter = new QSplitter(Qt::Horizontal, centralWidget);
    splitter->setHandleWidth(1);

    // Left Sidebar
    m_treeWidget = new QTreeWidget(splitter);
    m_treeWidget->setHeaderHidden(true);
    m_treeWidget->setMinimumWidth(210);
    m_treeWidget->setMaximumWidth(270);
    m_treeWidget->setIndentation(14);
    m_treeWidget->setIconSize(QSize(18, 18));

    // Shared Model
    m_tableModel = new FileTableModel(this);

    // Multi-View Stack (Icons, List, Columns)
    m_viewStack = new QStackedWidget(splitter);

    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);

    auto* layout = new QHBoxLayout(centralWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(splitter);

    // Bottom Path Breadcrumb Bar & Status Label
    m_pathBreadcrumb = new QLabel(this);
    m_statusLabel = new QLabel(this);
    statusBar()->addWidget(m_pathBreadcrumb, 1);
    statusBar()->addPermanentWidget(m_statusLabel);
}

void MainWindow::setupViews() {
    // -------------------------------------------------------------
    // View 0: macOS Icon Grid View (Large 76x76 icons with text below)
    // -------------------------------------------------------------
    m_iconView = new QListView(m_viewStack);
    m_iconView->setModel(m_tableModel);
    m_iconView->setModelColumn(FileTableModel::ColName);
    m_iconView->setViewMode(QListView::IconMode);
    m_iconView->setMovement(QListView::Static);
    m_iconView->setResizeMode(QListView::Adjust);
    m_iconView->setLayoutMode(QListView::Batched);
    m_iconView->setIconSize(QSize(76, 76));
    m_iconView->setGridSize(QSize(115, 115));
    m_iconView->setSpacing(12);
    m_iconView->setWordWrap(true);
    m_iconView->setTextElideMode(Qt::ElideMiddle);
    m_iconView->setUniformItemSizes(true);
    m_iconView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_iconView->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(m_iconView, &QListView::doubleClicked, this, &MainWindow::onEntryDoubleClicked);
    connect(m_iconView, &QListView::customContextMenuRequested, this, &MainWindow::showContextMenu);

    m_viewStack->addWidget(m_iconView);

    // -------------------------------------------------------------
    // View 1: macOS List Table View (Detailed table)
    // -------------------------------------------------------------
    m_tableView = new QTableView(m_viewStack);
    m_tableView->setModel(m_tableModel);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_tableView->setShowGrid(false);
    m_tableView->setAlternatingRowColors(false);
    m_tableView->setIconSize(QSize(22, 22));
    m_tableView->verticalHeader()->setVisible(false);
    m_tableView->verticalHeader()->setDefaultSectionSize(34);
    m_tableView->horizontalHeader()->setStretchLastSection(true);
    m_tableView->horizontalHeader()->setSectionResizeMode(FileTableModel::ColName, QHeaderView::Stretch);
    m_tableView->horizontalHeader()->setHighlightSections(false);

    connect(m_tableView, &QTableView::doubleClicked, this, &MainWindow::onEntryDoubleClicked);
    connect(m_tableView, &QTableView::customContextMenuRequested, this, &MainWindow::showContextMenu);

    m_viewStack->addWidget(m_tableView);

    // -------------------------------------------------------------
    // View 2: macOS Columns / Compact View (Medium 30px icons list)
    // -------------------------------------------------------------
    m_compactView = new QListView(m_viewStack);
    m_compactView->setModel(m_tableModel);
    m_compactView->setModelColumn(FileTableModel::ColName);
    m_compactView->setViewMode(QListView::ListMode);
    m_compactView->setFlow(QListView::TopToBottom);
    m_compactView->setResizeMode(QListView::Adjust);
    m_compactView->setIconSize(QSize(30, 30));
    m_compactView->setGridSize(QSize(240, 38));
    m_compactView->setSpacing(4);
    m_compactView->setWordWrap(false);
    m_compactView->setTextElideMode(Qt::ElideRight);
    m_compactView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_compactView->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(m_compactView, &QListView::doubleClicked, this, &MainWindow::onEntryDoubleClicked);
    connect(m_compactView, &QListView::customContextMenuRequested, this, &MainWindow::showContextMenu);

    m_viewStack->addWidget(m_compactView);

    // Default to Icon View (0)
    m_viewStack->setCurrentIndex(0);
}

void MainWindow::onSetViewModeIcons() {
    m_currentViewMode = 0;
    m_viewStack->setCurrentIndex(0);
    m_actViewIcons->setChecked(true);
    m_actViewList->setChecked(false);
    m_actViewCompact->setChecked(false);
}

void MainWindow::onSetViewModeList() {
    m_currentViewMode = 1;
    m_viewStack->setCurrentIndex(1);
    m_actViewIcons->setChecked(false);
    m_actViewList->setChecked(true);
    m_actViewCompact->setChecked(false);
}

void MainWindow::onSetViewModeCompact() {
    m_currentViewMode = 2;
    m_viewStack->setCurrentIndex(2);
    m_actViewIcons->setChecked(false);
    m_actViewList->setChecked(false);
    m_actViewCompact->setChecked(true);
}

void MainWindow::setupToolbars() {
    auto* navToolbar = addToolBar(QStringLiteral("Finder Toolbar"));
    navToolbar->setMovable(false);
    navToolbar->setIconSize(QSize(16, 16));

    // Navigation buttons (< > ^)
    auto* actBack = navToolbar->addAction(QStringLiteral("◀"));
    actBack->setToolTip(QStringLiteral("Back (Alt+Left)"));
    auto* actFwd = navToolbar->addAction(QStringLiteral("▶"));
    actFwd->setToolTip(QStringLiteral("Forward (Alt+Right)"));
    auto* actUp = navToolbar->addAction(QStringLiteral("▲"));
    actUp->setToolTip(QStringLiteral("Enclosing Folder (Up / Backspace)"));
    auto* actRefresh = navToolbar->addAction(QStringLiteral("⟳"));
    actRefresh->setToolTip(QStringLiteral("Reload (F5)"));

    connect(actBack, &QAction::triggered, this, &MainWindow::onNavigateBack);
    connect(actFwd, &QAction::triggered, this, &MainWindow::onNavigateForward);
    connect(actUp, &QAction::triggered, this, &MainWindow::onNavigateUp);
    connect(actRefresh, &QAction::triggered, this, &MainWindow::onRefresh);

    navToolbar->addSeparator();

    // macOS View Mode Segmented Controls (Icons, List, Columns)
    m_actViewIcons = navToolbar->addAction(QStringLiteral("▦ Icons"));
    m_actViewIcons->setToolTip(QStringLiteral("Icon Grid View (Ctrl+1)"));
    m_actViewIcons->setCheckable(true);
    m_actViewIcons->setChecked(true);
    connect(m_actViewIcons, &QAction::triggered, this, &MainWindow::onSetViewModeIcons);

    m_actViewList = navToolbar->addAction(QStringLiteral("☰ List"));
    m_actViewList->setToolTip(QStringLiteral("Detailed List View (Ctrl+2)"));
    m_actViewList->setCheckable(true);
    connect(m_actViewList, &QAction::triggered, this, &MainWindow::onSetViewModeList);

    m_actViewCompact = navToolbar->addAction(QStringLiteral("☷ Columns"));
    m_actViewCompact->setToolTip(QStringLiteral("Compact Columns View (Ctrl+3)"));
    m_actViewCompact->setCheckable(true);
    connect(m_actViewCompact, &QAction::triggered, this, &MainWindow::onSetViewModeCompact);

    navToolbar->addSeparator();

    // Address / Path Bar
    m_addressBar = new QLineEdit(this);
    m_addressBar->setObjectName(QStringLiteral("AddressBar"));
    m_addressBar->setPlaceholderText(QStringLiteral("Enter path..."));
    m_addressBar->setMinimumWidth(260);
    navToolbar->addWidget(m_addressBar);
    connect(m_addressBar, &QLineEdit::returnPressed, this, &MainWindow::onAddressEntered);

    navToolbar->addSeparator();

    // Quick Action Buttons
    auto* actNewFolder = navToolbar->addAction(QStringLiteral("＋ Folder"));
    connect(actNewFolder, &QAction::triggered, this, &MainWindow::onNewFolder);

    auto* actNewFile = navToolbar->addAction(QStringLiteral("＋ File"));
    connect(actNewFile, &QAction::triggered, this, &MainWindow::onNewFile);

    auto* actInfo = navToolbar->addAction(QStringLiteral("ℹ Info"));
    connect(actInfo, &QAction::triggered, this, &MainWindow::onShowProperties);

    auto* actDelete = navToolbar->addAction(QStringLiteral("🗑"));
    connect(actDelete, &QAction::triggered, this, &MainWindow::onDelete);

    navToolbar->addSeparator();

    // Spotlight Search Box
    m_searchBox = new QLineEdit(this);
    m_searchBox->setObjectName(QStringLiteral("SearchBox"));
    m_searchBox->setPlaceholderText(QStringLiteral("🔍 Search..."));
    m_searchBox->setMaximumWidth(190);
    navToolbar->addWidget(m_searchBox);
    connect(m_searchBox, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
}

void MainWindow::setupTree() {
    connect(m_treeWidget, &QTreeWidget::itemExpanded, this, &MainWindow::onTreeItemExpanded);
    connect(m_treeWidget, &QTreeWidget::itemClicked, this, &MainWindow::onTreeItemClicked);

    // Section 1: Favorites
    auto* favGroup = new QTreeWidgetItem(m_treeWidget);
    favGroup->setText(0, QStringLiteral("FAVORITES"));
    favGroup->setFlags(Qt::ItemIsEnabled);
    QFont f = favGroup->font(0);
    f.setBold(true);
    f.setPixelSize(11);
    favGroup->setFont(0, f);
    favGroup->setForeground(0, QBrush(QColor(140, 140, 145)));

    const char* userProf = std::getenv("USERPROFILE");
    if (!userProf) userProf = std::getenv("HOME");
    std::filesystem::path homePath = userProf ? std::filesystem::u8path(userProf) : std::filesystem::current_path();

    auto addFav = [this, favGroup](const QString& name, const std::filesystem::path& p, const QIcon& ico) {
        std::error_code ec;
        if (std::filesystem::exists(p, ec)) {
            auto* item = new QTreeWidgetItem(favGroup);
            item->setText(0, name);
            item->setData(0, Qt::UserRole, QString::fromStdString(p.u8string()));
            item->setIcon(0, ico);
        }
    };

    addFav(QStringLiteral("Home"), homePath, MacTheme::homeIcon());
    addFav(QStringLiteral("Desktop"), homePath / "Desktop", MacTheme::desktopIcon());
    addFav(QStringLiteral("Downloads"), homePath / "Downloads", MacTheme::downloadsIcon());
    addFav(QStringLiteral("Documents"), homePath / "Documents", MacTheme::documentsIcon());
    addFav(QStringLiteral("Pictures"), homePath / "Pictures", MacTheme::picturesIcon());
    addFav(QStringLiteral("Music"), homePath / "Music", MacTheme::musicIcon());
    addFav(QStringLiteral("Videos"), homePath / "Videos", MacTheme::videosIcon());

    favGroup->setExpanded(true);

    // Section 2: Locations (Real Windows Volume Names, e.g. ACER (C:))
    auto* locGroup = new QTreeWidgetItem(m_treeWidget);
    locGroup->setText(0, QStringLiteral("LOCATIONS"));
    locGroup->setFlags(Qt::ItemIsEnabled);
    locGroup->setFont(0, f);
    locGroup->setForeground(0, QBrush(QColor(140, 140, 145)));

#if defined(_WIN32)
    for (char drive = 'C'; drive <= 'Z'; ++drive) {
        std::string driveRoot = std::string(1, drive) + ":\\";
        std::error_code ec;
        if (std::filesystem::exists(driveRoot, ec)) {
            QString displayName = getDriveDisplayName(drive);
            auto* item = new QTreeWidgetItem(locGroup);
            item->setText(0, displayName);
            item->setData(0, Qt::UserRole, QString::fromStdString(driveRoot));
            item->setIcon(0, MacTheme::driveIcon());
            new QTreeWidgetItem(item); // Dummy for lazy child expansion
        }
    }
#else
    auto* rootItem = new QTreeWidgetItem(locGroup);
    rootItem->setText(0, QStringLiteral("Macintosh HD (/)"));
    rootItem->setData(0, Qt::UserRole, QStringLiteral("/"));
    rootItem->setIcon(0, MacTheme::driveIcon());
    new QTreeWidgetItem(rootItem);
#endif

    locGroup->setExpanded(true);
}

void MainWindow::setupPropertiesDock() {
    m_propertiesDock = new QDockWidget(QStringLiteral("Get Info"), this);
    m_propertiesDock->setAllowedAreas(Qt::RightDockWidgetArea | Qt::LeftDockWidgetArea);

    m_propertiesText = new QTextEdit(m_propertiesDock);
    m_propertiesText->setReadOnly(true);
    m_propertiesDock->setWidget(m_propertiesText);

    addDockWidget(Qt::RightDockWidgetArea, m_propertiesDock);
    m_propertiesDock->hide();
}

void MainWindow::setupShortcuts() {
    new QShortcut(QKeySequence(Qt::Key_Backspace), this, SLOT(onNavigateUp()));
    new QShortcut(QKeySequence(Qt::ALT | Qt::Key_Left), this, SLOT(onNavigateBack()));
    new QShortcut(QKeySequence(Qt::ALT | Qt::Key_Right), this, SLOT(onNavigateForward()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_1), this, SLOT(onSetViewModeIcons()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_2), this, SLOT(onSetViewModeList()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_3), this, SLOT(onSetViewModeCompact()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_I), this, SLOT(onShowProperties()));
    new QShortcut(QKeySequence(Qt::Key_Delete), this, SLOT(onDelete()));
    new QShortcut(QKeySequence(Qt::Key_F2), this, SLOT(onRename()));
    new QShortcut(QKeySequence(Qt::Key_F5), this, SLOT(onRefresh()));
    new QShortcut(QKeySequence::Copy, this, SLOT(onCopy()));
    new QShortcut(QKeySequence::Cut, this, SLOT(onCut()));
    new QShortcut(QKeySequence::Paste, this, SLOT(onPaste()));
}

const EntryInfo* MainWindow::getSelectedEntry() const {
    QModelIndex index;
    if (m_currentViewMode == 1 && m_tableView->selectionModel()) {
        auto sel = m_tableView->selectionModel()->selectedRows();
        if (!sel.isEmpty()) index = sel.first();
    } else if (m_currentViewMode == 0 && m_iconView->selectionModel()) {
        auto sel = m_iconView->selectionModel()->selectedIndexes();
        if (!sel.isEmpty()) index = sel.first();
    } else if (m_currentViewMode == 2 && m_compactView->selectionModel()) {
        auto sel = m_compactView->selectionModel()->selectedIndexes();
        if (!sel.isEmpty()) index = sel.first();
    }

    if (index.isValid()) {
        return m_tableModel->entryAt(index.row());
    }
    return nullptr;
}

void MainWindow::updateView() {
    const auto& curPath = m_navigator.current();
    std::string pathStr = curPath.u8string();
    m_addressBar->setText(QString::fromStdString(pathStr));

    // Update Header Window Title
    std::string folderName = curPath.filename().u8string();
    if (folderName.empty()) folderName = pathStr;
    m_titleLabel->setText(QStringLiteral("📁 %1").arg(QString::fromUtf8(folderName.c_str())));
    setWindowTitle(QStringLiteral("%1 — Finder").arg(QString::fromUtf8(folderName.c_str())));

    ListOptions opt;
    opt.showHidden = false;
    auto listRes = listDirectory(curPath, opt);
    if (!listRes.ok) {
        QMessageBox::warning(this, QStringLiteral("Finder"), QString::fromStdString(listRes.error));
        return;
    }

    m_tableModel->setEntries(listRes.value);

    // Build bottom breadcrumb with Windows Volume Label
    QString breadcrumb = QString::fromStdString(pathStr);
#if defined(_WIN32)
    if (pathStr.length() >= 2 && pathStr[1] == ':') {
        char drive = pathStr[0];
        QString driveLabel = getDriveDisplayName(drive);
        breadcrumb.replace(QStringLiteral("%1:\\").arg(drive), QStringLiteral("%1 ⟩ ").arg(driveLabel));
        breadcrumb.replace(QStringLiteral("%1:/").arg(drive), QStringLiteral("%1 ⟩ ").arg(driveLabel));
    }
#endif
    breadcrumb.replace('\\', QStringLiteral(" ⟩ "));
    breadcrumb.replace('/', QStringLiteral(" ⟩ "));
    m_pathBreadcrumb->setText(breadcrumb);

    // Update status counter and free space
    auto spRes = getSpace(curPath);
    QString statusMsg = QStringLiteral("%1 items").arg(listRes.value.size());
    if (spRes.ok) {
        statusMsg += QStringLiteral(", %1 available")
                         .arg(QString::fromStdString(formatSize(spRes.value.available)));
    }
    m_statusLabel->setText(statusMsg);
}

void MainWindow::populateTreeNode(QTreeWidgetItem* parentItem, const std::filesystem::path& dirPath) {
    parentItem->takeChildren();

    ListOptions opt;
    opt.showHidden = false;
    auto listRes = listDirectory(dirPath, opt);
    if (!listRes.ok) return;

    for (const auto& item : listRes.value) {
        if (item.type == EntryType::Directory) {
            auto* child = new QTreeWidgetItem(parentItem);
            child->setText(0, QString::fromUtf8(item.name.c_str()));
            child->setData(0, Qt::UserRole, QString::fromStdString(item.path.u8string()));
            child->setIcon(0, MacTheme::folderIcon(18));
            new QTreeWidgetItem(child);
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
        QMessageBox::warning(this, QStringLiteral("Finder"), QString::fromStdString(res.error));
    }
    updateView();
}

void MainWindow::onEntryDoubleClicked(const QModelIndex& index) {
    const auto* entry = m_tableModel->entryAt(index.row());
    if (!entry) return;

    if (entry->type == EntryType::Directory) {
        auto res = m_navigator.cd(entry->path.u8string());
        if (res.ok) {
            updateView();
        } else {
            QMessageBox::warning(this, QStringLiteral("Finder"), QString::fromStdString(res.error));
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
    QWidget* senderWidget = qobject_cast<QWidget*>(sender());
    QPoint globalPos = senderWidget ? senderWidget->mapToGlobal(pos) : QCursor::pos();

    QMenu menu(this);
    menu.addAction(QStringLiteral("New Folder"), this, &MainWindow::onNewFolder);
    menu.addAction(QStringLiteral("New File"), this, &MainWindow::onNewFile);
    menu.addSeparator();

    const auto* entry = getSelectedEntry();
    if (entry) {
        menu.addAction(QStringLiteral("Copy"), this, &MainWindow::onCopy);
        menu.addAction(QStringLiteral("Cut"), this, &MainWindow::onCut);
        menu.addAction(QStringLiteral("Rename..."), this, &MainWindow::onRename);
        menu.addAction(QStringLiteral("Move to Trash"), this, &MainWindow::onDelete);
        menu.addSeparator();
        menu.addAction(QStringLiteral("Get Info"), this, &MainWindow::onShowProperties);
    }

    if (!m_clipboardPath.empty()) {
        menu.addAction(QStringLiteral("Paste Item"), this, &MainWindow::onPaste);
    }

    menu.exec(globalPos);
}

void MainWindow::onNewFolder() {
    bool ok = false;
    QString name = QInputDialog::getText(this, QStringLiteral("New Folder"),
                                         QStringLiteral("Folder name:"), QLineEdit::Normal, QStringLiteral("untitled folder"), &ok);
    if (ok && !name.trimmed().isEmpty()) {
        auto res = createDirectory(m_navigator.current() / name.toStdString());
        if (!res.ok) {
            QMessageBox::warning(this, QStringLiteral("Finder"), QString::fromStdString(res.error));
        } else {
            updateView();
        }
    }
}

void MainWindow::onNewFile() {
    bool ok = false;
    QString name = QInputDialog::getText(this, QStringLiteral("New File"),
                                         QStringLiteral("File name:"), QLineEdit::Normal, QStringLiteral("untitled.txt"), &ok);
    if (ok && !name.trimmed().isEmpty()) {
        auto res = createFile(m_navigator.current() / name.toStdString());
        if (!res.ok) {
            QMessageBox::warning(this, QStringLiteral("Finder"), QString::fromStdString(res.error));
        } else {
            updateView();
        }
    }
}

void MainWindow::onRename() {
    const auto* entry = getSelectedEntry();
    if (!entry) return;

    bool ok = false;
    QString newName = QInputDialog::getText(this, QStringLiteral("Rename"),
                                            QStringLiteral("New name:"), QLineEdit::Normal,
                                            QString::fromUtf8(entry->name.c_str()), &ok);
    if (ok && !newName.trimmed().isEmpty()) {
        auto res = renamePath(entry->path, newName.toStdString());
        if (!res.ok) {
            QMessageBox::warning(this, QStringLiteral("Finder"), QString::fromStdString(res.error));
        } else {
            updateView();
        }
    }
}

void MainWindow::onDelete() {
    const auto* entry = getSelectedEntry();
    if (!entry) return;

    auto reply = QMessageBox::question(this, QStringLiteral("Finder"),
                                       QStringLiteral("Are you sure you want to delete \"%1\"?")
                                           .arg(QString::fromUtf8(entry->name.c_str())),
                                       QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        auto res = removePath(entry->path, true);
        if (!res.ok) {
            QMessageBox::warning(this, QStringLiteral("Finder"), QString::fromStdString(res.error));
        } else {
            updateView();
        }
    }
}

void MainWindow::onCopy() {
    const auto* entry = getSelectedEntry();
    if (entry) {
        m_clipboardPath = entry->path;
        m_clipboardIsCut = false;
    }
}

void MainWindow::onCut() {
    const auto* entry = getSelectedEntry();
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
        QMessageBox::warning(this, QStringLiteral("Finder"), QString::fromStdString(res.error));
    } else {
        updateView();
    }
}

void MainWindow::onShowProperties() {
    const auto* entry = getSelectedEntry();
    if (!entry) return;

    auto infoRes = getInfo(entry->path);
    if (!infoRes.ok) return;

    const auto& info = infoRes.value;
    QString iconEmoji = (info.type == EntryType::Directory) ? QStringLiteral("📁") : QStringLiteral("📄");
    QString typeLabel = (info.type == EntryType::Directory) ? QStringLiteral("Folder") : QStringLiteral("Document");

    QString propHtml = QStringLiteral(
        "<div style='font-family:-apple-system, sans-serif; padding:10px;'>"
        "<div style='text-align:center; margin-bottom:14px;'>"
        "<span style='font-size:42px;'>%1</span>"
        "<h3 style='margin:6px 0 2px 0; font-size:16px;'>%2</h3>"
        "<span style='color:#86868B; font-size:12px;'>%3 — %4</span>"
        "</div>"
        "<hr style='border:0; border-top:1px solid rgba(140,140,145,0.25); margin:12px 0;'/>"
        "<table style='width:100%%; font-size:12px; line-height:22px;'>"
        "<tr><td style='color:#86868B; width:90px;'><b>Kind:</b></td><td>%3</td></tr>"
        "<tr><td style='color:#86868B;'><b>Size:</b></td><td>%4 (%5 bytes)</td></tr>"
        "<tr><td style='color:#86868B;'><b>Where:</b></td><td style='word-break:break-all;'>%6</td></tr>"
        "<tr><td style='color:#86868B;'><b>Modified:</b></td><td>%7</td></tr>"
        "<tr><td style='color:#86868B;'><b>Permissions:</b></td><td><code>%8</code></td></tr>"
        "<tr><td style='color:#86868B;'><b>Hidden:</b></td><td>%9</td></tr>"
        "</table>"
        "</div>")
        .arg(iconEmoji)
        .arg(QString::fromUtf8(info.name.c_str()))
        .arg(typeLabel)
        .arg(QString::fromStdString(formatSize(info.size)))
        .arg(info.size)
        .arg(QString::fromStdString(info.path.u8string()))
        .arg(QString::fromStdString(formatTime(info.modified)))
        .arg(QString::fromStdString(formatPermissions(info.permissions)))
        .arg(info.isHidden ? QStringLiteral("Yes") : QStringLiteral("No"));

    m_propertiesText->setHtml(propHtml);
    m_propertiesDock->show();
    m_propertiesDock->raise();
}

} // namespace fx::gui
