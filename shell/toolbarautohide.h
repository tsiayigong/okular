/*
    SPDX-FileCopyrightText: 2026 tsiayigong
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef OKULAR_TOOLBARAUTOHIDE_H
#define OKULAR_TOOLBARAUTOHIDE_H

#include <QObject>
#include <QPointer>
#include <QTimer>

class QMainWindow;
class QToolBar;
class QWidget;

class ToolbarAutoHide : public QObject
{
    Q_OBJECT
public:
    explicit ToolbarAutoHide(QMainWindow *window);
    void attach(QToolBar *toolbar, QWidget *document);
    void setEnabled(bool enabled);
    bool isEnabled() const;
    void reveal(bool focus = false);

protected:
    bool eventFilter(QObject *object, QEvent *event) override;

private:
    void updateGeometry();
    void hideIfIdle();
    bool isInteracting() const;

    QMainWindow *m_window;
    QPointer<QToolBar> m_toolbar;
    QPointer<QWidget> m_document;
    QWidget *m_hotZone;
    QWidget *m_handle;
    QTimer m_hideTimer;
    Qt::ToolBarArea m_area = Qt::TopToolBarArea;
    bool m_enabled = false;
};

#endif
