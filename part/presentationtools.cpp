/*
    SPDX-FileCopyrightText: 2026 tsiayigong
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "presentationtools.h"
#include "drawingtoolactions.h"

#include <KIconColors>
#include <KIconLoader>
#include <KLocalizedString>
#include <QAction>
#include <QActionGroup>
#include <QMenu>
#include <QPainter>
#include <QToolBar>
#include <QToolButton>

PresentationTools::PresentationTools(DrawingToolActions *presets, QObject *parent)
    : QObject(parent)
    , m_presets(presets)
    , m_tools(new QActionGroup(this))
    , m_colors(new QActionGroup(this))
    , m_widths(new QActionGroup(this))
    , m_undo(new QAction(QIcon::fromTheme(QStringLiteral("edit-undo")), i18n("Undo Last Stroke"), this))
    , m_clear(new QAction(QIcon::fromTheme(QStringLiteral("draw-eraser-delete-objects")), i18n("Clear Page Ink"), this))
{
    const QStringList labels = {i18n("Pointer"), i18n("Laser Pointer"), i18n("Pen"), i18nc("Presentation tool", "Highlighter"), i18n("Eraser")};
    const QStringList icons = {QStringLiteral("edit-select"), QString(), QStringLiteral("draw-freehand"), QStringLiteral("draw-highlight"), QStringLiteral("draw-eraser")};
    const QStringList names = {QStringLiteral("pointer"), QStringLiteral("laser"), QStringLiteral("pen"), QStringLiteral("highlighter"), QStringLiteral("eraser")};
    const KIconColors controlsColors(QColor(QStringLiteral("#d3d9e2")));
    for (int index = Pointer; index <= Eraser; ++index) {
        const QIcon icon = index == Laser ? QIcon(QStringLiteral(":/okular/icons/presentation-laser.svg")) : KDE::icon(icons[index], controlsColors);
        auto *action = m_tools->addAction(icon, labels[index]);
        action->setObjectName(QStringLiteral("presentation_") + names[index]);
        action->setCheckable(true);
        action->setData(index);
        action->setChecked(index == Pointer);
    }
    connect(m_tools, &QActionGroup::triggered, this, [this](QAction *action) { setTool(static_cast<Tool>(action->data().toInt())); });

    const QStringList colors = {QStringLiteral("#e6535e"), QStringLiteral("#f5bf3a"), QStringLiteral("#9ad469"), QStringLiteral("#4badef"), QStringLiteral("#bc93e7"), QStringLiteral("#f6f7f9")};
    for (const QString &name : colors) {
        QPixmap swatch(40, 40);
        swatch.setDevicePixelRatio(2);
        swatch.fill(Qt::transparent);
        QPainter painter(&swatch);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(QPen(QColor(QStringLiteral("#808890")), 0.5));
        painter.setBrush(QColor(name));
        painter.drawEllipse(QRectF(2, 2, 16, 16));
        painter.end();
        auto *action = m_colors->addAction(QIcon(swatch), i18n("Ink Color: %1", name));
        action->setCheckable(true);
        action->setData(QColor(name));
        action->setChecked(QColor(name) == m_color);
    }
    connect(m_colors, &QActionGroup::triggered, this, [this](QAction *action) {
        (m_tool == Highlighter ? m_highlighterColor : m_color) = action->data().value<QColor>();
        setTool(m_tool == Custom ? Pen : m_tool);
    });
    for (const int width : {2, 3, 6}) {
        auto *action = m_widths->addAction(i18n("%1 px", width));
        action->setData(width);
        action->setCheckable(true);
        action->setChecked(width == m_width);
    }
    connect(m_widths, &QActionGroup::triggered, this, [this](QAction *action) {
        m_width = action->data().toInt();
        setTool(m_tool == Custom ? Pen : m_tool);
    });
    m_undo->setObjectName(QStringLiteral("presentation_undo_ink"));
    m_clear->setObjectName(QStringLiteral("presentation_clear_ink"));
    m_undo->setShortcut(QKeySequence::Undo);
    m_undo->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(m_undo, &QAction::triggered, this, &PresentationTools::undoRequested);
    connect(m_clear, &QAction::triggered, this, &PresentationTools::clearRequested);
    connect(m_presets, &DrawingToolActions::changeEngine, this, [this](const QDomElement &engine) {
        if (engine.isNull()) {
            setTool(Pointer);
        } else if (engine.attribute(QStringLiteral("compositionMode")) == QLatin1String("clear")) {
            setTool(Eraser);
        } else {
            m_customEngine = engine;
            m_tool = Custom;
            m_tools->setExclusive(false);
            for (auto *action : m_tools->actions()) {
                action->setChecked(false);
            }
            m_tools->setExclusive(true);
            Q_EMIT changed();
        }
    });
    setInkActionsEnabled(false, false);
}

PresentationTools::~PresentationTools()
{
    for (auto *action : m_presets->actions()) {
        action->setChecked(false);
        action->setEnabled(false);
    }
}

void PresentationTools::addToToolBar(QToolBar *bar)
{
    bar->addActions(m_tools->actions());
    bar->addSeparator();
    bar->addActions(m_colors->actions());
    auto *options = new QMenu(bar);
    options->addSection(i18n("Stroke Width"));
    options->addActions(m_widths->actions());
    options->addSeparator();
    options->addAction(m_undo);
    options->addAction(m_clear);
    auto *presets = options->addMenu(i18n("Drawing Presets"));
    const auto populatePresets = [this, presets] {
        presets->clear();
        for (auto *action : m_presets->actions()) {
            action->setEnabled(true);
            presets->addAction(action);
        }
    };
    populatePresets();
    connect(m_presets, &DrawingToolActions::actionsRecreated, presets, populatePresets);
    auto *button = new QToolButton(bar);
    button->setIcon(KDE::icon(QStringLiteral("overflow-menu"), KIconColors(QColor(QStringLiteral("#d3d9e2")))));
    button->setToolTip(i18n("Stroke and Ink"));
    button->setMenu(options);
    button->setPopupMode(QToolButton::InstantPopup);
    bar->addWidget(button);
    bar->window()->addAction(m_undo);
}

PresentationTools::Tool PresentationTools::tool() const
{
    return m_tool;
}

QColor PresentationTools::color() const
{
    return m_tool == Highlighter ? m_highlighterColor : m_color;
}

void PresentationTools::setTool(Tool tool)
{
    m_tool = tool;
    for (auto *action : m_presets->actions()) {
        action->setChecked(false);
    }
    for (auto *action : m_tools->actions()) {
        action->setChecked(action->data().toInt() == tool);
    }
    for (auto *action : m_colors->actions()) {
        action->setChecked(action->data().value<QColor>() == color());
    }
    Q_EMIT changed();
}

QDomElement PresentationTools::drawingEngine() const
{
    if (m_tool == Custom) {
        return m_customEngine;
    }
    if (m_tool != Pen && m_tool != Highlighter) {
        return QDomElement();
    }
    QDomDocument document;
    auto engine = document.createElement(QStringLiteral("engine"));
    document.appendChild(engine);
    engine.setAttribute(QStringLiteral("color"), color().name());
    auto annotation = document.createElement(QStringLiteral("annotation"));
    annotation.setAttribute(QStringLiteral("type"), QStringLiteral("Ink"));
    annotation.setAttribute(QStringLiteral("color"), color().name());
    annotation.setAttribute(QStringLiteral("width"), m_tool == Highlighter ? m_width * 7 : m_width);
    annotation.setAttribute(QStringLiteral("opacity"), m_tool == Highlighter ? 0.35 : 1.0);
    engine.appendChild(annotation);
    return engine;
}

void PresentationTools::setInkActionsEnabled(bool canUndo, bool hasInk)
{
    m_undo->setEnabled(canUndo);
    m_clear->setEnabled(hasInk);
}
