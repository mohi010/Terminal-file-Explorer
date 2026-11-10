// ============================================================================
// File: src/gui/mac_theme.cpp
// Purpose: Implementations for macOS Finder icons, vector graphics, and stylesheets.
// ============================================================================

#include "mac_theme.hpp"
#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>

namespace fx::gui {

QIcon MacTheme::folderIcon(int size) {
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter p(&pixmap);
    p.setRenderHint(QPainter::Antialiasing);

    // Coordinate scaling
    qreal s = static_cast<qreal>(size);
    qreal unit = s / 32.0;

    // 1. Back folder body
    QPainterPath backPath;
    backPath.addRoundedRect(QRectF(3 * unit, 6 * unit, 26 * unit, 21 * unit), 3 * unit, 3 * unit);
    QLinearGradient backGrad(0, 6 * unit, 0, 27 * unit);
    backGrad.setColorAt(0.0, QColor(0, 122, 255));
    backGrad.setColorAt(1.0, QColor(0, 90, 200));
    p.setPen(Qt::NoPen);
    p.setBrush(backGrad);
    p.drawPath(backPath);

    // 2. Folder Tab
    QPainterPath tabPath;
    tabPath.moveTo(3 * unit, 9 * unit);
    tabPath.lineTo(3 * unit, 6 * unit);
    tabPath.quadTo(3 * unit, 4 * unit, 6 * unit, 4 * unit);
    tabPath.lineTo(13 * unit, 4 * unit);
    tabPath.quadTo(15 * unit, 4 * unit, 17 * unit, 7 * unit);
    tabPath.lineTo(17 * unit, 9 * unit);
    tabPath.closeSubpath();
    p.setBrush(QColor(0, 115, 240));
    p.drawPath(tabPath);

    // 3. Front Flap (macOS signature bright cyan-blue gradient)
    QPainterPath frontPath;
    frontPath.moveTo(2 * unit, 11 * unit);
    frontPath.quadTo(2 * unit, 9 * unit, 5 * unit, 9 * unit);
    frontPath.lineTo(27 * unit, 9 * unit);
    frontPath.quadTo(30 * unit, 9 * unit, 30 * unit, 11 * unit);
    frontPath.lineTo(29 * unit, 26 * unit);
    frontPath.quadTo(29 * unit, 28 * unit, 26 * unit, 28 * unit);
    frontPath.lineTo(6 * unit, 28 * unit);
    frontPath.quadTo(3 * unit, 28 * unit, 3 * unit, 26 * unit);
    frontPath.closeSubpath();

    QLinearGradient frontGrad(0, 9 * unit, 0, 28 * unit);
    frontGrad.setColorAt(0.0, QColor(64, 180, 255));
    frontGrad.setColorAt(0.4, QColor(25, 145, 255));
    frontGrad.setColorAt(1.0, QColor(0, 110, 235));
    p.setBrush(frontGrad);
    p.setPen(QPen(QColor(10, 90, 200, 120), 0.8 * unit));
    p.drawPath(frontPath);

    // 4. Subtle glossy top highlight on front flap
    QPainterPath highlightPath;
    highlightPath.moveTo(4 * unit, 10 * unit);
    highlightPath.lineTo(28 * unit, 10 * unit);
    p.setPen(QPen(QColor(255, 255, 255, 150), 0.8 * unit));
    p.drawPath(highlightPath);

    return QIcon(pixmap);
}

QIcon MacTheme::fileIcon(const QString& extension, int size) {
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter p(&pixmap);
    p.setRenderHint(QPainter::Antialiasing);

    qreal s = static_cast<qreal>(size);
    qreal unit = s / 32.0;

    // Main document body with folded top-right corner
    qreal foldSize = 7.0 * unit;
    QPainterPath docPath;
    docPath.moveTo(6 * unit, 3 * unit);
    docPath.lineTo((26 * unit) - foldSize, 3 * unit);
    docPath.lineTo(26 * unit, 3 * unit + foldSize);
    docPath.lineTo(26 * unit, 28 * unit);
    docPath.quadTo(26 * unit, 29 * unit, 24 * unit, 29 * unit);
    docPath.lineTo(8 * unit, 29 * unit);
    docPath.quadTo(6 * unit, 29 * unit, 6 * unit, 28 * unit);
    docPath.closeSubpath();

    // Document fill
    QLinearGradient docGrad(0, 3 * unit, 0, 29 * unit);
    docGrad.setColorAt(0.0, QColor(255, 255, 255));
    docGrad.setColorAt(1.0, QColor(240, 242, 245));
    p.setPen(QPen(QColor(180, 185, 192), 0.9 * unit));
    p.setBrush(docGrad);
    p.drawPath(docPath);

    // Folded corner flap
    QPainterPath foldPath;
    foldPath.moveTo((26 * unit) - foldSize, 3 * unit);
    foldPath.lineTo((26 * unit) - foldSize, 3 * unit + foldSize);
    foldPath.lineTo(26 * unit, 3 * unit + foldSize);
    foldPath.closeSubpath();
    p.setBrush(QColor(215, 220, 228));
    p.setPen(QPen(QColor(180, 185, 192), 0.8 * unit));
    p.drawPath(foldPath);

    // Horizontal document text preview lines
    p.setPen(QPen(QColor(190, 195, 205), 1.2 * unit));
    p.drawLine(QPointF(9 * unit, 12 * unit), QPointF(20 * unit, 12 * unit));
    p.drawLine(QPointF(9 * unit, 15 * unit), QPointF(23 * unit, 15 * unit));
    p.drawLine(QPointF(9 * unit, 18 * unit), QPointF(21 * unit, 18 * unit));

    // Colored badge tag for extension (e.g. cpp, h, txt, py)
    QString ext = extension.toLower();
    if (ext.startsWith(QLatin1Char('.'))) ext = ext.mid(1);

    QColor badgeColor(0, 122, 255); // Default blue
    if (ext == QStringLiteral("cpp") || ext == QStringLiteral("c") || ext == QStringLiteral("hpp") || ext == QStringLiteral("h")) {
        badgeColor = QColor(255, 95, 86); // Red/Orange for C++
    } else if (ext == QStringLiteral("py") || ext == QStringLiteral("pyw")) {
        badgeColor = QColor(52, 199, 89); // Green for Python
    } else if (ext == QStringLiteral("md") || ext == QStringLiteral("txt") || ext == QStringLiteral("doc")) {
        badgeColor = QColor(0, 122, 255); // Blue for docs
    } else if (ext == QStringLiteral("json") || ext == QStringLiteral("xml") || ext == QStringLiteral("yaml")) {
        badgeColor = QColor(255, 149, 0); // Orange for data
    } else if (ext == QStringLiteral("exe") || ext == QStringLiteral("dll") || ext == QStringLiteral("a")) {
        badgeColor = QColor(175, 82, 222); // Purple for binaries
    }

    if (!ext.isEmpty()) {
        QRectF badgeRect(8 * unit, 21 * unit, 16 * unit, 6 * unit);
        p.setPen(Qt::NoPen);
        p.setBrush(badgeColor);
        p.drawRoundedRect(badgeRect, 2 * unit, 2 * unit);

        QFont f = p.font();
        f.setPixelSize(static_cast<int>(3.6 * unit));
        f.setBold(true);
        p.setFont(f);
        p.setPen(Qt::white);
        p.drawText(badgeRect, Qt::AlignCenter, ext.left(4).toUpper());
    }

    return QIcon(pixmap);
}

QIcon MacTheme::driveIcon(int size) {
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter p(&pixmap);
    p.setRenderHint(QPainter::Antialiasing);

    qreal s = static_cast<qreal>(size);
    qreal unit = s / 32.0;

    // Aluminum drive case
    QPainterPath drivePath;
    drivePath.addRoundedRect(QRectF(4 * unit, 8 * unit, 24 * unit, 16 * unit), 3 * unit, 3 * unit);
    QLinearGradient driveGrad(0, 8 * unit, 0, 24 * unit);
    driveGrad.setColorAt(0.0, QColor(220, 223, 228));
    driveGrad.setColorAt(0.5, QColor(190, 195, 202));
    driveGrad.setColorAt(1.0, QColor(160, 166, 175));
    p.setPen(QPen(QColor(130, 135, 145), 0.8 * unit));
    p.setBrush(driveGrad);
    p.drawPath(drivePath);

    // Front plate
    QPainterPath frontPlate;
    frontPlate.addRoundedRect(QRectF(6 * unit, 15 * unit, 20 * unit, 7 * unit), 1.5 * unit, 1.5 * unit);
    p.setBrush(QColor(70, 75, 82));
    p.setPen(Qt::NoPen);
    p.drawPath(frontPlate);

    // LED status light (green)
    p.setBrush(QColor(52, 199, 89));
    p.drawEllipse(QRectF(22 * unit, 17.5 * unit, 2 * unit, 2 * unit));

    // Subtle horizontal disk slot
    p.setPen(QPen(QColor(120, 125, 135), 1.0 * unit));
    p.drawLine(QPointF(8 * unit, 18.5 * unit), QPointF(19 * unit, 18.5 * unit));

    return QIcon(pixmap);
}

QIcon MacTheme::homeIcon(int size) {
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter p(&pixmap);
    p.setRenderHint(QPainter::Antialiasing);

    qreal s = static_cast<qreal>(size);
    qreal unit = s / 32.0;

    // Blue house shape
    QPainterPath roof;
    roof.moveTo(16 * unit, 5 * unit);
    roof.lineTo(5 * unit, 15 * unit);
    roof.lineTo(8 * unit, 15 * unit);
    roof.lineTo(8 * unit, 26 * unit);
    roof.lineTo(24 * unit, 26 * unit);
    roof.lineTo(24 * unit, 15 * unit);
    roof.lineTo(27 * unit, 15 * unit);
    roof.closeSubpath();

    QLinearGradient grad(0, 5 * unit, 0, 26 * unit);
    grad.setColorAt(0.0, QColor(0, 140, 255));
    grad.setColorAt(1.0, QColor(0, 100, 220));
    p.setPen(Qt::NoPen);
    p.setBrush(grad);
    p.drawPath(roof);

    // Doorway
    QPainterPath door;
    door.addRoundedRect(QRectF(13 * unit, 18 * unit, 6 * unit, 8 * unit), 1 * unit, 1 * unit);
    p.setBrush(Qt::white);
    p.drawPath(door);

    return QIcon(pixmap);
}

QIcon MacTheme::desktopIcon(int size) {
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter p(&pixmap);
    p.setRenderHint(QPainter::Antialiasing);

    qreal s = static_cast<qreal>(size);
    qreal unit = s / 32.0;

    // iMac/Display body
    QPainterPath screen;
    screen.addRoundedRect(QRectF(4 * unit, 5 * unit, 24 * unit, 16 * unit), 2 * unit, 2 * unit);
    p.setPen(QPen(QColor(0, 100, 220), 0.8 * unit));
    p.setBrush(QColor(0, 122, 255));
    p.drawPath(screen);

    // Screen inner glass
    QPainterPath inner;
    inner.addRect(QRectF(6 * unit, 7 * unit, 20 * unit, 11 * unit));
    p.setBrush(QColor(230, 242, 255));
    p.drawPath(inner);

    // Stand
    QPainterPath stand;
    stand.moveTo(14 * unit, 21 * unit);
    stand.lineTo(13 * unit, 26 * unit);
    stand.lineTo(19 * unit, 26 * unit);
    stand.lineTo(18 * unit, 21 * unit);
    stand.closeSubpath();
    p.setBrush(QColor(160, 170, 185));
    p.setPen(Qt::NoPen);
    p.drawPath(stand);

    return QIcon(pixmap);
}

QIcon MacTheme::documentsIcon(int size) {
    return fileIcon(QStringLiteral("doc"), size);
}

QIcon MacTheme::downloadsIcon(int size) {
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter p(&pixmap);
    p.setRenderHint(QPainter::Antialiasing);

    qreal s = static_cast<qreal>(size);
    qreal unit = s / 32.0;

    // Circle background in Apple blue
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 122, 255));
    p.drawEllipse(QRectF(4 * unit, 4 * unit, 24 * unit, 24 * unit));

    // White down arrow and tray
    QPainterPath arrow;
    arrow.moveTo(16 * unit, 8 * unit);
    arrow.lineTo(16 * unit, 18 * unit);
    arrow.moveTo(11 * unit, 14 * unit);
    arrow.lineTo(16 * unit, 19 * unit);
    arrow.lineTo(21 * unit, 14 * unit);
    p.setPen(QPen(Qt::white, 2.2 * unit, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.drawPath(arrow);

    p.drawLine(QPointF(10 * unit, 23 * unit), QPointF(22 * unit, 23 * unit));

    return QIcon(pixmap);
}

QIcon MacTheme::picturesIcon(int size) {
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter p(&pixmap);
    p.setRenderHint(QPainter::Antialiasing);

    qreal s = static_cast<qreal>(size);
    qreal unit = s / 32.0;

    // Picture frame (Teal / Cyan)
    QPainterPath frame;
    frame.addRoundedRect(QRectF(4 * unit, 5 * unit, 24 * unit, 20 * unit), 3 * unit, 3 * unit);
    QLinearGradient grad(0, 5 * unit, 0, 25 * unit);
    grad.setColorAt(0.0, QColor(0, 190, 210));
    grad.setColorAt(1.0, QColor(0, 140, 180));
    p.setPen(Qt::NoPen);
    p.setBrush(grad);
    p.drawPath(frame);

    // Sun / Moon
    p.setBrush(QColor(255, 220, 90));
    p.drawEllipse(QRectF(8 * unit, 8 * unit, 4 * unit, 4 * unit));

    // Mountain silhouettes
    QPainterPath mountains;
    mountains.moveTo(4 * unit, 23 * unit);
    mountains.lineTo(12 * unit, 14 * unit);
    mountains.lineTo(17 * unit, 19 * unit);
    mountains.lineTo(21 * unit, 13 * unit);
    mountains.lineTo(28 * unit, 23 * unit);
    mountains.closeSubpath();
    p.setBrush(QColor(255, 255, 255, 220));
    p.drawPath(mountains);

    return QIcon(pixmap);
}

QIcon MacTheme::musicIcon(int size) {
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter p(&pixmap);
    p.setRenderHint(QPainter::Antialiasing);

    qreal s = static_cast<qreal>(size);
    qreal unit = s / 32.0;

    // Circle background (Apple Music Red/Pink)
    QLinearGradient grad(0, 4 * unit, 0, 28 * unit);
    grad.setColorAt(0.0, QColor(255, 59, 92));
    grad.setColorAt(1.0, QColor(220, 20, 60));
    p.setPen(Qt::NoPen);
    p.setBrush(grad);
    p.drawEllipse(QRectF(4 * unit, 4 * unit, 24 * unit, 24 * unit));

    // Eighth note pair
    QPainterPath note;
    note.addEllipse(QRectF(8 * unit, 19 * unit, 5 * unit, 4 * unit));
    note.addEllipse(QRectF(19 * unit, 16 * unit, 5 * unit, 4 * unit));
    p.setBrush(Qt::white);
    p.drawPath(note);

    p.setPen(QPen(Qt::white, 2 * unit, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.drawLine(QPointF(12.5 * unit, 20 * unit), QPointF(12.5 * unit, 10 * unit));
    p.drawLine(QPointF(23.5 * unit, 17 * unit), QPointF(23.5 * unit, 7 * unit));
    p.drawLine(QPointF(12.5 * unit, 10 * unit), QPointF(23.5 * unit, 7 * unit));

    return QIcon(pixmap);
}

QIcon MacTheme::videosIcon(int size) {
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter p(&pixmap);
    p.setRenderHint(QPainter::Antialiasing);

    qreal s = static_cast<qreal>(size);
    qreal unit = s / 32.0;

    // Movie clapperboard / TV screen (Purple / Indigo)
    QPainterPath screen;
    screen.addRoundedRect(QRectF(4 * unit, 6 * unit, 24 * unit, 18 * unit), 3 * unit, 3 * unit);
    QLinearGradient grad(0, 6 * unit, 0, 24 * unit);
    grad.setColorAt(0.0, QColor(160, 90, 240));
    grad.setColorAt(1.0, QColor(110, 50, 200));
    p.setPen(Qt::NoPen);
    p.setBrush(grad);
    p.drawPath(screen);

    // Play triangle
    QPainterPath play;
    play.moveTo(13 * unit, 11 * unit);
    play.lineTo(21 * unit, 15 * unit);
    play.lineTo(13 * unit, 19 * unit);
    play.closeSubpath();
    p.setBrush(Qt::white);
    p.drawPath(play);

    return QIcon(pixmap);
}

QIcon MacTheme::trashIcon(int size) {
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter p(&pixmap);
    p.setRenderHint(QPainter::Antialiasing);

    qreal s = static_cast<qreal>(size);
    qreal unit = s / 32.0;

    // Trash can body
    QPainterPath body;
    body.moveTo(9 * unit, 10 * unit);
    body.lineTo(11 * unit, 26 * unit);
    body.quadTo(11 * unit, 28 * unit, 13 * unit, 28 * unit);
    body.lineTo(19 * unit, 28 * unit);
    body.quadTo(21 * unit, 28 * unit, 21 * unit, 26 * unit);
    body.lineTo(23 * unit, 10 * unit);
    body.closeSubpath();

    p.setPen(QPen(QColor(140, 145, 155), 1.0 * unit));
    p.setBrush(QColor(230, 233, 238));
    p.drawPath(body);

    // Lid
    p.drawLine(QPointF(7 * unit, 9 * unit), QPointF(25 * unit, 9 * unit));
    p.drawLine(QPointF(13 * unit, 6 * unit), QPointF(19 * unit, 6 * unit));

    // Vertical ridges
    p.drawLine(QPointF(13 * unit, 13 * unit), QPointF(13.5 * unit, 24 * unit));
    p.drawLine(QPointF(16 * unit, 13 * unit), QPointF(16 * unit, 24 * unit));
    p.drawLine(QPointF(19 * unit, 13 * unit), QPointF(18.5 * unit, 24 * unit));

    return QIcon(pixmap);
}

QIcon MacTheme::searchIcon(int size) {
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter p(&pixmap);
    p.setRenderHint(QPainter::Antialiasing);

    qreal s = static_cast<qreal>(size);
    qreal unit = s / 32.0;

    // Magnifying glass lens
    p.setPen(QPen(QColor(130, 135, 145), 2.2 * unit));
    p.setBrush(Qt::NoBrush);
    p.drawEllipse(QRectF(7 * unit, 7 * unit, 12 * unit, 12 * unit));

    // Handle
    p.drawLine(QPointF(16 * unit, 16 * unit), QPointF(25 * unit, 25 * unit));

    return QIcon(pixmap);
}

QIcon MacTheme::closeButtonIcon(int size) {
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(QColor(224, 68, 62), 1.0));
    p.setBrush(QColor(255, 95, 86));
    p.drawEllipse(1, 1, size - 3, size - 3);
    return QIcon(pixmap);
}

QIcon MacTheme::minimizeButtonIcon(int size) {
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(QColor(222, 161, 35), 1.0));
    p.setBrush(QColor(255, 189, 46));
    p.drawEllipse(1, 1, size - 3, size - 3);
    return QIcon(pixmap);
}

QIcon MacTheme::maximizeButtonIcon(int size) {
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(QColor(26, 171, 41), 1.0));
    p.setBrush(QColor(39, 201, 63));
    p.drawEllipse(1, 1, size - 3, size - 3);
    return QIcon(pixmap);
}

QString MacTheme::getMacStylesheet(bool darkMode) {
    if (darkMode) {
        return QStringLiteral(R"(
            /* =======================================================
               macOS Sonoma / Sequoia Finder Theme (Dark Mode)
               ======================================================= */
            * {
                font-family: -apple-system, BlinkMacSystemFont, "SF Pro Text", "Segoe UI", Roboto, "Helvetica Neue", Arial, sans-serif;
                font-size: 13px;
                color: #ECECEC;
            }

            QMainWindow {
                background-color: #1E1E20;
            }

            /* Custom macOS Top Header */
            #MacHeader {
                background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #323236, stop:1 #28282B);
                border-bottom: 1px solid #141416;
                padding: 6px 14px;
            }

            #MacTitleLabel {
                font-size: 13px;
                font-weight: 600;
                color: #FFFFFF;
            }

            /* Traffic Lights */
            QToolButton#TrafficClose, QToolButton#TrafficMin, QToolButton#TrafficMax {
                border: none;
                border-radius: 6px;
                background: transparent;
                min-width: 12px;
                min-height: 12px;
                max-width: 12px;
                max-height: 12px;
                margin-right: 6px;
            }

            /* Toolbar */
            QToolBar {
                background: #252528;
                border-bottom: 1px solid #18181A;
                spacing: 6px;
                padding: 6px 12px;
            }

            /* macOS Segmented Buttons & Toolbar Buttons */
            QToolBar QToolButton, QPushButton {
                background-color: #38383C;
                border: 1px solid #48484C;
                border-radius: 6px;
                padding: 4px 10px;
                font-weight: 500;
                color: #F0F0F2;
            }
            QToolBar QToolButton:hover, QPushButton:hover {
                background-color: #48484E;
                border-color: #5A5A60;
            }
            QToolBar QToolButton:pressed, QPushButton:pressed {
                background-color: #28282C;
            }

            /* Address / Path Bar */
            QLineEdit#AddressBar {
                background-color: #1E1E22;
                border: 1px solid #3A3A40;
                border-radius: 7px;
                padding: 4px 10px;
                color: #E2E2E6;
                selection-background-color: #0A84FF;
                font-family: "SF Mono", "Segoe UI Mono", Menlo, Consolas, monospace;
                font-size: 12px;
            }
            QLineEdit#AddressBar:focus {
                border: 1px solid #0A84FF;
                background-color: #161618;
            }

            /* Search Box (macOS Spotlight Style) */
            QLineEdit#SearchBox {
                background-color: #2E2E32;
                border: 1px solid #3E3E44;
                border-radius: 14px;
                padding: 4px 12px;
                color: #FFFFFF;
                selection-background-color: #0A84FF;
            }
            QLineEdit#SearchBox:focus {
                border: 1px solid #0A84FF;
                background-color: #242428;
            }

            /* Finder Sidebar */
            QTreeWidget {
                background-color: #212124;
                border: none;
                border-right: 1px solid #18181A;
                padding: 8px 4px;
                outline: none;
            }
            QTreeWidget::item {
                height: 28px;
                border-radius: 6px;
                margin: 1px 6px;
                padding-left: 4px;
                color: #D1D1D6;
                font-weight: 450;
            }
            QTreeWidget::item:hover {
                background-color: rgba(255, 255, 255, 0.08);
            }
            QTreeWidget::item:selected {
                background-color: #0A84FF;
                color: #FFFFFF;
                font-weight: 600;
            }

            /* Main File Table (macOS List View) */
            QTableView {
                background-color: #1A1A1C;
                border: none;
                gridline-color: transparent;
                selection-background-color: #0A84FF;
                selection-color: #FFFFFF;
                outline: none;
            }
            QTableView::item {
                padding: 4px 8px;
                border-bottom: 1px solid #242428;
            }
            QTableView::item:selected {
                background-color: #0A84FF;
                color: #FFFFFF;
            }
            QTableView::item:hover:!selected {
                background-color: rgba(255, 255, 255, 0.04);
            }

            /* macOS Icon Grid View */
            QListView {
                background-color: #1A1A1C;
                border: none;
                outline: none;
                padding: 12px;
            }
            QListView::item {
                border-radius: 8px;
                padding: 8px 4px;
                margin: 4px;
                color: #ECECEC;
            }
            QListView::item:hover {
                background-color: rgba(255, 255, 255, 0.08);
            }
            QListView::item:selected {
                background-color: rgba(10, 132, 255, 0.35);
                border: 1px solid #0A84FF;
                color: #FFFFFF;
            }

            /* Table Header */
            QHeaderView::section {
                background-color: #212124;
                color: #8E8E93;
                font-size: 11px;
                font-weight: 600;
                text-transform: uppercase;
                border: none;
                border-bottom: 1px solid #2A2A2E;
                padding: 6px 8px;
            }

            /* Splitter */
            QSplitter::handle {
                background-color: #18181A;
                width: 1px;
            }

            /* Status Bar / Bottom Path Bar */
            QStatusBar {
                background-color: #222225;
                border-top: 1px solid #18181A;
                color: #8E8E93;
                font-size: 11px;
                padding: 3px 12px;
            }

            /* macOS Get Info Dock Panel */
            QDockWidget {
                titlebar-close-icon: url();
                titlebar-normal-icon: url();
                border: none;
            }
            QDockWidget::title {
                background: #252528;
                border-bottom: 1px solid #1C1C1E;
                padding: 8px 12px;
                font-weight: 600;
                color: #FFFFFF;
            }
            QTextEdit {
                background-color: #1C1C1E;
                border: none;
                padding: 12px;
                color: #ECECEC;
            }

            /* Scrollbars */
            QScrollBar:vertical {
                background: transparent;
                width: 10px;
                margin: 0;
            }
            QScrollBar::handle:vertical {
                background: rgba(255, 255, 255, 0.2);
                border-radius: 5px;
                min-height: 24px;
                margin: 2px;
            }
            QScrollBar::handle:vertical:hover {
                background: rgba(255, 255, 255, 0.4);
            }
            QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
                height: 0;
            }

            /* Context Menu */
            QMenu {
                background-color: #2C2C2E;
                border: 1px solid #444448;
                border-radius: 8px;
                padding: 6px 4px;
            }
            QMenu::item {
                padding: 6px 20px 6px 12px;
                border-radius: 5px;
                color: #F2F2F7;
            }
            QMenu::item:selected {
                background-color: #0A84FF;
                color: #FFFFFF;
            }
            QMenu::separator {
                height: 1px;
                background: #444448;
                margin: 4px 8px;
            }
        )");
    } else {
        // macOS Sonoma Light Mode
        return QStringLiteral(R"(
            /* =======================================================
               macOS Sonoma / Sequoia Finder Theme (Light Mode)
               ======================================================= */
            * {
                font-family: -apple-system, BlinkMacSystemFont, "SF Pro Text", "Segoe UI", Roboto, "Helvetica Neue", Arial, sans-serif;
                font-size: 13px;
                color: #1D1D1F;
            }

            QMainWindow {
                background-color: #F6F6F6;
            }

            /* Custom macOS Top Header */
            #MacHeader {
                background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #EBEBEB, stop:1 #E0E0E0);
                border-bottom: 1px solid #C8C8C8;
                padding: 6px 14px;
            }

            #MacTitleLabel {
                font-size: 13px;
                font-weight: 600;
                color: #222222;
            }

            /* Traffic Lights */
            QToolButton#TrafficClose, QToolButton#TrafficMin, QToolButton#TrafficMax {
                border: none;
                border-radius: 6px;
                background: transparent;
                min-width: 12px;
                min-height: 12px;
                max-width: 12px;
                max-height: 12px;
                margin-right: 6px;
            }

            /* Toolbar */
            QToolBar {
                background: #F4F4F6;
                border-bottom: 1px solid #D6D6DC;
                spacing: 6px;
                padding: 6px 12px;
            }

            /* macOS Segmented Buttons & Toolbar Buttons */
            QToolBar QToolButton, QPushButton {
                background-color: #FFFFFF;
                border: 1px solid #D1D1D6;
                border-radius: 6px;
                padding: 4px 10px;
                font-weight: 500;
                color: #1D1D1F;
            }
            QToolBar QToolButton:hover, QPushButton:hover {
                background-color: #F2F2F7;
                border-color: #B0B0B8;
            }
            QToolBar QToolButton:pressed, QPushButton:pressed {
                background-color: #E5E5EA;
            }

            /* Address / Path Bar */
            QLineEdit#AddressBar {
                background-color: #FFFFFF;
                border: 1px solid #D1D1D6;
                border-radius: 7px;
                padding: 4px 10px;
                color: #1D1D1F;
                selection-background-color: #007AFF;
                font-family: "SF Mono", "Segoe UI Mono", Menlo, Consolas, monospace;
                font-size: 12px;
            }
            QLineEdit#AddressBar:focus {
                border: 1px solid #007AFF;
                background-color: #FFFFFF;
            }

            /* Search Box (macOS Spotlight Style) */
            QLineEdit#SearchBox {
                background-color: #EAEAEA;
                border: 1px solid #D6D6DC;
                border-radius: 14px;
                padding: 4px 12px;
                color: #1D1D1F;
                selection-background-color: #007AFF;
            }
            QLineEdit#SearchBox:focus {
                border: 1px solid #007AFF;
                background-color: #FFFFFF;
            }

            /* Finder Sidebar */
            QTreeWidget {
                background-color: #EDECEF;
                border: none;
                border-right: 1px solid #D6D6DC;
                padding: 8px 4px;
                outline: none;
            }
            QTreeWidget::item {
                height: 28px;
                border-radius: 6px;
                margin: 1px 6px;
                padding-left: 4px;
                color: #3C3C43;
                font-weight: 450;
            }
            QTreeWidget::item:hover {
                background-color: rgba(0, 0, 0, 0.05);
            }
            QTreeWidget::item:selected {
                background-color: #007AFF;
                color: #FFFFFF;
                font-weight: 600;
            }

            /* Main File Table (macOS List View) */
            QTableView {
                background-color: #FFFFFF;
                border: none;
                gridline-color: transparent;
                selection-background-color: #007AFF;
                selection-color: #FFFFFF;
                outline: none;
            }
            QTableView::item {
                padding: 4px 8px;
                border-bottom: 1px solid #F0F0F2;
            }
            QTableView::item:selected {
                background-color: #007AFF;
                color: #FFFFFF;
            }
            QTableView::item:hover:!selected {
                background-color: rgba(0, 0, 0, 0.03);
            }

            /* macOS Icon Grid View */
            QListView {
                background-color: #FFFFFF;
                border: none;
                outline: none;
                padding: 12px;
            }
            QListView::item {
                border-radius: 8px;
                padding: 8px 4px;
                margin: 4px;
                color: #1D1D1F;
            }
            QListView::item:hover {
                background-color: rgba(0, 0, 0, 0.05);
            }
            QListView::item:selected {
                background-color: rgba(0, 122, 255, 0.15);
                border: 1px solid #007AFF;
                color: #007AFF;
                font-weight: 600;
            }

            /* Table Header */
            QHeaderView::section {
                background-color: #FAFAFA;
                color: #86868B;
                font-size: 11px;
                font-weight: 600;
                text-transform: uppercase;
                border: none;
                border-bottom: 1px solid #E5E5EA;
                padding: 6px 8px;
            }

            /* Splitter */
            QSplitter::handle {
                background-color: #D6D6DC;
                width: 1px;
            }

            /* Status Bar / Bottom Path Bar */
            QStatusBar {
                background-color: #F2F2F4;
                border-top: 1px solid #D6D6DC;
                color: #86868B;
                font-size: 11px;
                padding: 3px 12px;
            }

            /* macOS Get Info Dock Panel */
            QDockWidget {
                border: none;
            }
            QDockWidget::title {
                background: #F4F4F6;
                border-bottom: 1px solid #E5E5EA;
                padding: 8px 12px;
                font-weight: 600;
                color: #1D1D1F;
            }
            QTextEdit {
                background-color: #FFFFFF;
                border: none;
                padding: 12px;
                color: #1D1D1F;
            }

            /* Scrollbars */
            QScrollBar:vertical {
                background: transparent;
                width: 10px;
                margin: 0;
            }
            QScrollBar::handle:vertical {
                background: rgba(0, 0, 0, 0.2);
                border-radius: 5px;
                min-height: 24px;
                margin: 2px;
            }
            QScrollBar::handle:vertical:hover {
                background: rgba(0, 0, 0, 0.35);
            }
            QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
                height: 0;
            }

            /* Context Menu */
            QMenu {
                background-color: #FFFFFF;
                border: 1px solid #D1D1D6;
                border-radius: 8px;
                padding: 6px 4px;
            }
            QMenu::item {
                padding: 6px 20px 6px 12px;
                border-radius: 5px;
                color: #1D1D1F;
            }
            QMenu::item:selected {
                background-color: #007AFF;
                color: #FFFFFF;
            }
            QMenu::separator {
                height: 1px;
                background: #E5E5EA;
                margin: 4px 8px;
            }
        )");
    }
}

} // namespace fx::gui

