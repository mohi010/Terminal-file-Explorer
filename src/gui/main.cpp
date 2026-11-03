// ============================================================================
// File: src/gui/main.cpp
// Purpose: Entry point for the Qt Widgets GUI application.
// OS Concept: Window system event loop and native desktop presentation.
// ============================================================================

#include <QApplication>
#include "mainwindow.hpp"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("OS File Explorer"));
    app.setOrganizationName(QStringLiteral("NSUT"));

    fx::gui::MainWindow window;
    window.show();

    return app.exec();
}

