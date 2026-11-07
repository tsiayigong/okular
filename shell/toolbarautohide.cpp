/*
    SPDX-FileCopyrightText: 2026 tsiayigong
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "toolbarautohide.h"

#include <QApplication>
#include <QEvent>
#include <QKeyEvent>
#include <QMainWindow>
#include <QToolBar>
#include <QWidget>

ToolbarAutoHide::ToolbarAutoHide(QMainWindow *window)
    : QObject(window)
    , m_window(window)
    , m_hotZone(new QWidget(window))
    , m_handle(new QWidget(m_hotZone))
{
    m_hotZone->setObjectName(QStringLiteral("toolbarRevealArea"));
    m_hotZone->setAccessibleName(tr("Show toolbar"));
    m_hotZone->setStyleSheet(QStringLiteral("background: transparent;"));
    m_hotZone->hide();
    m_handle->setFixedSize(30, 2);
    m_handle->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_handle->setStyleSheet(QStringLiteral("background: #94A0AC; border-radius: 1px;"));
    m_hideTimer.setInterval(260);
    m_hideTimer.setSingleShot(true);
    connect(&m_hideTimer, &QTimer::timeout, this, &ToolbarAutoHide::hideIfIdle);
    qApp->installEventFilter(this);
}

void ToolbarAutoHide::attach(QToolBar *toolbar, QWidget *document)
{
    if (m_toolbar == toolbar && m_document == document) {
        updateGeometry();
        return;
    }
    const bool enabled = m_enabled;
    setEnabled(false);
    m_toolbar = toolbar;
    m_document = document;
    setEnabled(enabled);
}

void ToolbarAutoHide::setEnabled(bool enabled)
{
    // XMLGUI can restore a docked layout while the preference remains enabled.
    if (m_enabled == enabled && (!enabled || !m_toolbar || m_window->toolBarArea(m_toolbar) == Qt::NoToolBarArea)) {
        return;
    }
    m_enabled = enabled;
    m_hideTimer.stop();
    if (!m_toolbar) {
        m_hotZone->hide();
        return;
    }
    if (enabled) {
        const auto area = m_window->toolBarArea(m_toolbar);
        if (area != Qt::NoToolBarArea) {
            m_area = area;
        }
        m_window->removeToolBar(m_toolbar);
        m_toolbar->hide();
        updateGeometry();
    } else {
        m_hotZone->hide();
        m_window->addToolBar(m_area, m_toolbar);
        m_toolbar->show();
    }
}

bool ToolbarAutoHide::isEnabled() const
{
    return m_enabled;
}

void ToolbarAutoHide::updateGeometry()
{
    if (!m_enabled || !m_toolbar || !m_document) {
        m_hotZone->hide();
        return;
    }
    const QPoint topLeft = m_document->mapTo(m_window, QPoint(0, 0));
    m_hotZone->setGeometry(QRect(topLeft, QSize(m_document->width(), 8)));
    m_handle->move((m_hotZone->width() - m_handle->width()) / 2, 3);
    m_hotZone->setVisible(m_document->isVisible());
    m_toolbar->setGeometry(QRect(topLeft, QSize(m_document->width(), m_toolbar->sizeHint().height())));
    m_hotZone->raise();
    if (m_toolbar->isVisible()) {
        m_toolbar->raise();
    }
}

void ToolbarAutoHide::reveal(bool focus)
{
    if (!m_enabled || !m_toolbar || !m_document || !m_document->isVisible()) {
        return;
    }
    m_hideTimer.stop();
    updateGeometry();
    m_toolbar->show();
    m_toolbar->raise();
    if (focus) {
        const auto widgets = m_toolbar->findChildren<QWidget *>();
        for (QWidget *widget : widgets) {
            if (widget->isVisible() && widget->isEnabled() && (widget->focusPolicy() & Qt::TabFocus)) {
                widget->setFocus(Qt::ShortcutFocusReason);
                break;
            }
        }
    }
}

bool ToolbarAutoHide::isInteracting() const
{
    if (!m_toolbar) {
        return false;
    }
    QWidget *focus = QApplication::focusWidget();
    return m_toolbar->underMouse() || m_hotZone->underMouse() || QApplication::activePopupWidget()
        || (focus && (focus == m_toolbar || m_toolbar->isAncestorOf(focus)));
}

void ToolbarAutoHide::hideIfIdle()
{
    if (!m_enabled || !m_toolbar) {
        return;
    }
    if (isInteracting()) {
        m_hideTimer.start();
    } else {
        m_toolbar->hide();
    }
}

bool ToolbarAutoHide::eventFilter(QObject *object, QEvent *event)
{
    if (!m_enabled) {
        return false;
    }
    switch (event->type()) {
    case QEvent::KeyPress:
        if (static_cast<QKeyEvent *>(event)->key() == Qt::Key_Escape && m_toolbar && m_document && m_toolbar->isAncestorOf(qobject_cast<QWidget *>(object))) {
            m_document->setFocus(Qt::ShortcutFocusReason);
            m_toolbar->hide();
            return true;
        }
        break;
    case QEvent::Enter:
        if (object == m_hotZone || object == m_toolbar) {
            reveal();
        }
        break;
    case QEvent::Leave:
    case QEvent::FocusOut:
        if (object == m_hotZone || object == m_toolbar || (m_toolbar && m_toolbar->isAncestorOf(qobject_cast<QWidget *>(object)))) {
            m_hideTimer.start();
        }
        break;
    case QEvent::Resize:
    case QEvent::Move:
    case QEvent::Show:
    case QEvent::Hide:
        if (object == m_document || object == m_window) {
            updateGeometry();
        }
        break;
    default:
        break;
    }
    return false;
}
