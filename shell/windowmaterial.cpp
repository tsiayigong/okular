/*
    SPDX-FileCopyrightText: 2026 tsiayigong
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "windowmaterial.h"

#include <QApplication>
#include <QDebug>
#include <QDialog>
#include <QEvent>
#include <QMenu>
#include <QWindow>

#ifdef Q_OS_WIN
#include <qt_windows.h>
#include <dwmapi.h>
#endif

WindowMaterial::WindowMaterial(QObject *parent)
    : QObject(parent)
{
#ifdef Q_OS_WIN
    qApp->installEventFilter(this);
#endif
}

bool WindowMaterial::eventFilter(QObject *object, QEvent *event)
{
    auto *widget = qobject_cast<QWidget *>(object);
    if (!widget || !widget->isWindow()) {
        return false;
    }
    switch (event->type()) {
    case QEvent::Polish:
        if (qobject_cast<QMenu *>(widget) || qobject_cast<QDialog *>(widget) || widget->objectName().startsWith(QLatin1String("okular::Shell"))) {
            widget->setProperty("nativeAcrylic", true);
            // Keep a native DWM frame so the raster backing store reaches the
            // compositor with its alpha channel, rather than a layered window.
            widget->setWindowFlag(Qt::FramelessWindowHint, false);
            widget->setAttribute(Qt::WA_TranslucentBackground);
        }
        break;
    case QEvent::Show:
    case QEvent::WinIdChange:
    case QEvent::PaletteChange:
        if (widget->property("nativeAcrylic").toBool() && widget->internalWinId()) {
            apply(widget);
        }
        break;
    default:
        break;
    }
    return false;
}

void WindowMaterial::apply(QWidget *widget)
{
#ifdef Q_OS_WIN
    const HWND hwnd = reinterpret_cast<HWND>(widget->internalWinId());
    const BOOL dark = widget->palette().color(QPalette::Window).lightness() < 128;
    const DWM_SYSTEMBACKDROP_TYPE backdrop = DWMSBT_TRANSIENTWINDOW;
    const DWM_WINDOW_CORNER_PREFERENCE corners = qobject_cast<QMenu *>(widget) ? DWMWCP_ROUNDSMALL : DWMWCP_ROUND;
    const MARGINS margins{-1, -1, -1, -1};
    DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &dark, sizeof(dark));
    DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &corners, sizeof(corners));
    const HRESULT frameResult = DwmExtendFrameIntoClientArea(hwnd, &margins);
    const HRESULT materialResult = DwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &backdrop, sizeof(backdrop));
    if (FAILED(frameResult) || FAILED(materialResult)) {
        qWarning() << "Windows acrylic requires Windows 11 build 22621 or later:" << frameResult << materialResult;
    }
    if (widget->property("unifiedTitleBar").toBool()) {
        widget->windowHandle()->setFlag(Qt::WindowTitleHint, false);
    }
#else
    Q_UNUSED(widget)
#endif
}
