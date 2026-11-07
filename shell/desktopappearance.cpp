/*
    SPDX-FileCopyrightText: 2026 tsiayigong
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "desktopappearance.h"
#include "windowmaterial.h"

#include <KIconLoader>
#include <QApplication>
#include <QPalette>
#include <QStyleHints>

DesktopAppearance *DesktopAppearance::instance()
{
    static auto *appearance = new DesktopAppearance(qApp);
    return appearance;
}

DesktopAppearance::DesktopAppearance(QObject *parent)
    : QObject(parent)
{
    QApplication::setStyle(QStringLiteral("Fusion"));
    new WindowMaterial(this);
    connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged, this, [this] {
        if (m_theme == 0) {
            apply();
        }
    });
}

void DesktopAppearance::setTheme(int theme, const QColor &accent)
{
    if (m_theme != theme || m_accent != accent) {
        m_theme = theme;
        m_accent = accent;
        QGuiApplication::styleHints()->setColorScheme(theme == 0 ? Qt::ColorScheme::Unknown : theme == 2 ? Qt::ColorScheme::Dark : Qt::ColorScheme::Light);
        apply();
    }
}

void DesktopAppearance::apply()
{
    const bool dark = m_theme == 2 || (m_theme == 0 && QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark);
    const QColor surface(dark ? "#292E36" : "#F4F6F8");
    const QColor field(dark ? "#20242B" : "#FCFCFD");
    const QColor text(dark ? "#F0F2F5" : "#252D38");
    const QColor muted(dark ? "#969FAB" : "#7C8592");
    const QColor accent = m_accent;
    QPalette palette;
    palette.setColor(QPalette::Window, surface);
    palette.setColor(QPalette::WindowText, text);
    palette.setColor(QPalette::Base, field);
    palette.setColor(QPalette::AlternateBase, surface);
    palette.setColor(QPalette::Text, text);
    palette.setColor(QPalette::Button, surface);
    palette.setColor(QPalette::ButtonText, text);
    palette.setColor(QPalette::Highlight, accent);
    palette.setColor(QPalette::HighlightedText, Qt::white);
    palette.setColor(QPalette::Link, accent);
    palette.setColor(QPalette::ToolTipBase, surface);
    palette.setColor(QPalette::ToolTipText, text);
    palette.setColor(QPalette::PlaceholderText, muted);
    for (const auto role : {QPalette::Text, QPalette::WindowText, QPalette::ButtonText}) {
        palette.setColor(QPalette::Disabled, role, muted);
    }
    QApplication::setPalette(palette);
    KIconLoader::global()->setCustomPalette(palette);

    QString css = QStringLiteral(R"(
        QMenuBar { background: @chrome; padding: 0px 8px; spacing: 2px; border-bottom: 1px solid @line; }
        QMenuBar::item { background: transparent; padding: 4px 9px; border-radius: 4px; }
        QMenuBar::item:selected { background: @hover; }
        QMenu { background: @surface; color: @text; border: 1px solid @line; border-radius: 6px; padding: 5px; }
        QMenu::item { padding: 5px 28px 5px 24px; border-radius: 4px; }
        QMenu::item:selected { background: #0A84FF; color: white; }
        QMenu::item:disabled { color: @muted; }
        QMenu::separator { height: 1px; background: @line; margin: 4px 8px; }
        QToolBar { background: @chrome; border: 0; border-bottom: 1px solid @line; spacing: 4px; padding: 4px 8px; }
        QToolBar::separator { width: 1px; background: @line; margin: 4px 6px; }
        QToolBar#documentTabBar { padding: 0px 8px; spacing: 4px; }
        QToolButton { border: 1px solid transparent; border-radius: 4px; padding: 4px; }
        QToolButton:hover { background: @hover; }
        QToolButton:checked { background: @selected; border-color: @line; }
        QTabWidget::pane { border: 0; }
        QTabBar { background: @chrome; }
        QTabBar::tab { background: transparent; color: @muted; border: 1px solid transparent; padding: 5px 12px; min-width: 90px; max-width: 260px; }
        QTabBar::tab:selected { background: @surface; color: @text; border-color: @line; border-bottom-color: @surface; border-top-left-radius: 4px; border-top-right-radius: 4px; }
        QTabBar::tab:hover:!selected { background: @hover; }
        QTabBar#sidebarTabs { background: @sidebar; border: 1px solid @line; border-radius: 4px; margin: 8px; }
        QTabBar#sidebarTabs::tab { min-width: 0; padding: 4px 8px; margin: 2px; border: 0; border-radius: 3px; }
        QTabBar#sidebarTabs::tab:selected { background: @field; }
        QDockWidget#okular_sidebar { background: @sidebar; border-right: 1px solid @line; }
        QDockWidget#okular_sidebar QAbstractScrollArea { background: @sidebar; border: 0; }
        QPushButton { background: @field; border: 1px solid @line; border-radius: 4px; padding: 5px 12px; }
        QPushButton:hover { background: @hover; }
        QPushButton:default { background: #0A84FF; border-color: #0A84FF; color: white; }
        QPushButton:disabled { color: @muted; }
        QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox { background: @field; color: @text; border: 1px solid @line; border-radius: 4px; padding: 4px 6px; }
        QLineEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QComboBox:focus { border-color: #0A84FF; }
        QComboBox { padding-right: 22px; }
        QComboBox::drop-down { border: 0; width: 20px; }
        QDialog { background: @surface; }
        QFrame#textSelectionActions { background: @surface; border: 1px solid @line; border-radius: 6px; }
        QFrame#AnnotWindow { background: @surface; border: 1px solid @line; border-radius: 6px; }
        QFrame#AnnotWindow QTextEdit, QFrame#AnnotWindow QPlainTextEdit { background: @field; border: 1px solid @line; border-radius: 4px; padding: 5px; }
        QStatusBar { background: @chrome; color: @muted; border-top: 1px solid @line; }
        QStatusBar::item { border: 0; }
        QToolTip { background: @surface; color: @text; border: 1px solid @line; border-radius: 4px; padding: 5px 8px; }
        QWidget#welcomeScreen { background: transparent; }
        QWidget#welcomeScreen QListView { background: transparent; border: 0; }
        QWidget#welcomeScreen QFrame#divider { color: @line; }
        QWidget#welcomeScreen QLabel#welcomeDescription, QWidget#welcomeScreen QLabel#openShortcut { color: @muted; }
        QWidget#welcomeScreen QLabel#documentDropArea, QWidget#welcomeScreen QLabel#emptyDropArea { border: 1px dashed @line; border-radius: 4px; color: @muted; padding: 8px; }
        QMainWindow[nativeAcrylic="true"] { background: transparent; }
        QMainWindow[nativeAcrylic="true"] QMenuBar, QMainWindow[nativeAcrylic="true"] QToolBar,
        QMainWindow[nativeAcrylic="true"] QTabBar, QMainWindow[nativeAcrylic="true"] QStatusBar { background: @glassChrome; }
        QMainWindow[nativeAcrylic="true"] QDockWidget#okular_sidebar,
        QMainWindow[nativeAcrylic="true"] QDockWidget#okular_sidebar QAbstractScrollArea { background: @glassSidebar; }
        QMenu[nativeAcrylic="true"], QDialog[nativeAcrylic="true"] { background: @glassSurface; }
    )");
    css.replace(QLatin1String("@surface"), surface.name());
    css.replace(QLatin1String("#0A84FF"), accent.name());
    css.replace(QLatin1String("@field"), field.name());
    css.replace(QLatin1String("@text"), text.name());
    css.replace(QLatin1String("@muted"), muted.name());
    css.replace(QLatin1String("@chrome"), dark ? QLatin1String("#2D3540") : QLatin1String("#DCE5EC"));
    css.replace(QLatin1String("@sidebar"), dark ? QLatin1String("#303B46") : QLatin1String("#D4E2EA"));
    css.replace(QLatin1String("@line"), dark ? QLatin1String("#454E5A") : QLatin1String("#D2DAE2"));
    css.replace(QLatin1String("@hover"), dark ? QLatin1String("#414C5A") : QLatin1String("#E5EBF1"));
    css.replace(QLatin1String("@selected"), dark ? QLatin1String("#284B71") : QLatin1String("#D9EBFF"));
    css.replace(QLatin1String("@glassChrome"), dark ? QLatin1String("rgba(38,45,54,155)") : QLatin1String("rgba(218,231,240,125)"));
    css.replace(QLatin1String("@glassSidebar"), dark ? QLatin1String("rgba(42,53,65,150)") : QLatin1String("rgba(207,226,237,140)"));
    css.replace(QLatin1String("@glassSurface"), dark ? QLatin1String("rgba(39,44,52,200)") : QLatin1String("rgba(247,249,252,190)"));
    qApp->setStyleSheet(css);
}
