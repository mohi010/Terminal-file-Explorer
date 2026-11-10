// ============================================================================
// File: src/gui/mac_theme.hpp
// Purpose: macOS aesthetic styles, vector icon generators, and color palettes.
// ============================================================================

#pragma once

#include <QIcon>
#include <QColor>
#include <QString>
#include <QPixmap>

namespace fx::gui {

class MacTheme {
public:
    // Core macOS Vector Icons generated with crisp anti-aliased QPainter
    static QIcon folderIcon(int size = 48);
    static QIcon fileIcon(const QString& extension = QString(), int size = 48);
    static QIcon driveIcon(int size = 48);
    static QIcon homeIcon(int size = 48);
    static QIcon desktopIcon(int size = 48);
    static QIcon documentsIcon(int size = 48);
    static QIcon downloadsIcon(int size = 48);
    static QIcon picturesIcon(int size = 48);
    static QIcon musicIcon(int size = 48);
    static QIcon videosIcon(int size = 48);
    static QIcon trashIcon(int size = 48);
    static QIcon searchIcon(int size = 32);
    static QIcon backIcon(int size = 32);
    static QIcon forwardIcon(int size = 32);
    static QIcon upIcon(int size = 32);
    static QIcon refreshIcon(int size = 32);
    static QIcon newFolderIcon(int size = 32);
    static QIcon newFileIcon(int size = 32);
    static QIcon infoIcon(int size = 32);
    static QIcon deleteIcon(int size = 32);

    // Traffic light button icons
    static QIcon closeButtonIcon(int size = 16);
    static QIcon minimizeButtonIcon(int size = 16);
    static QIcon maximizeButtonIcon(int size = 16);

    // Full macOS Finder QSS Stylesheet (Dark & Light)
    static QString getMacStylesheet(bool darkMode = true);
};

} // namespace fx::gui

