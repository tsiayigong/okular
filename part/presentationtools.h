/*
    SPDX-FileCopyrightText: 2026 tsiayigong
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef OKULAR_PRESENTATIONTOOLS_H
#define OKULAR_PRESENTATIONTOOLS_H

#include <QColor>
#include <QDomElement>
#include <QObject>

class QAction;
class QActionGroup;
class QToolBar;
class DrawingToolActions;

class PresentationTools : public QObject
{
    Q_OBJECT
public:
    enum Tool { Pointer, Laser, Pen, Highlighter, Eraser, Custom };

    explicit PresentationTools(DrawingToolActions *presets, QObject *parent);
    ~PresentationTools() override;
    void addToToolBar(QToolBar *bar);
    Tool tool() const;
    QColor color() const;
    QDomElement drawingEngine() const;
    void setTool(Tool tool);
    void setInkActionsEnabled(bool canUndo, bool hasInk);

Q_SIGNALS:
    void changed();
    void undoRequested();
    void clearRequested();

private:
    DrawingToolActions *m_presets;
    QActionGroup *m_tools;
    QActionGroup *m_colors;
    QActionGroup *m_widths;
    QAction *m_undo;
    QAction *m_clear;
    Tool m_tool = Pointer;
    QColor m_color = QColor(QStringLiteral("#e6535e"));
    QColor m_highlighterColor = QColor(QStringLiteral("#f5bf3a"));
    int m_width = 3;
    QDomElement m_customEngine;
};

#endif
