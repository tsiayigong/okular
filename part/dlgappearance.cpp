/*
    SPDX-FileCopyrightText: 2026 tsiayigong
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "dlgappearance.h"

#include <KLocalizedString>
#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QLabel>

DlgAppearance::DlgAppearance(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QFormLayout(this);
    layout->setContentsMargins(20, 16, 20, 16);
    layout->setVerticalSpacing(16);

    auto *theme = new QComboBox(this);
    theme->setObjectName(QStringLiteral("kcfg_InterfaceTheme"));
    theme->addItems({i18n("Follow system"), i18n("Light"), i18n("Dark")});
    layout->addRow(i18n("Appearance:"), theme);

    auto *autoHide = new QCheckBox(i18n("Automatically hide the main toolbar"), this);
    autoHide->setObjectName(QStringLiteral("kcfg_AutoHideMainToolBar"));
    layout->addRow(autoHide);
    auto *hint = new QLabel(i18n("Move the pointer to the top edge of the document to show the toolbar. Press Ctrl+Alt+T to focus it."), this);
    hint->setWordWrap(true);
    layout->addRow(hint);

    auto *paper = new QLabel(i18n("Paper color applies to both the document and its thumbnails. Adjust it in Accessibility → Change paper color."), this);
    paper->setWordWrap(true);
    layout->addRow(paper);
}
