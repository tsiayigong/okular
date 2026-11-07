/*
    SPDX-FileCopyrightText: 2006 Pino Toscano <toscano.pino@tiscali.it>
    SPDX-FileCopyrightText: 2026 tsiayigong
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "dlggeneral.h"

#include <KAuthorized>
#include <KColorButton>
#include <KLocalizedString>
#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QSpinBox>
#include <QVBoxLayout>
#include <config-okular.h>

DlgGeneral::DlgGeneral(QWidget *parent, Okular::EmbedMode embedMode)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 8, 24, 16);
    layout->setSpacing(16);

    auto *opening = new QGroupBox(i18n("Opening Documents"), this);
    auto *openingLayout = new QFormLayout(opening);
    openingLayout->setVerticalSpacing(12);
    if (embedMode == Okular::NativeShellMode) {
        auto *useTabs = new QCheckBox(i18nc("@option:check Config dialog, general page", "Open new files in tabs"), this);
        useTabs->setObjectName(QStringLiteral("kcfg_ShellOpenFileInTabs"));
        openingLayout->addRow(useTabs);
        auto *switchToTab = new QCheckBox(i18nc("@option:check Config dialog, general page", "Switch to existing tab if file is already open"), this);
        switchToTab->setObjectName(QStringLiteral("kcfg_SwitchToTabIfOpen"));
        switchToTab->setEnabled(false);
        openingLayout->addRow(switchToTab);
        connect(useTabs, &QCheckBox::toggled, switchToTab, &QWidget::setEnabled);
    }
    auto *continueReading = new QCheckBox(i18nc("@option:check Config dialog, general page", "Continue from last viewed page"), this);
    continueReading->setObjectName(QStringLiteral("kcfg_ContinueFromLastViewedPage"));
    openingLayout->addRow(continueReading);
    auto *maxRecentItems = new QSpinBox(this);
    maxRecentItems->setObjectName(QStringLiteral("kcfg_MaxRecentItems"));
    openingLayout->addRow(i18nc("@label:spinbox Config dialog, general page", "Items in recent history:"), maxRecentItems);
    layout->addWidget(opening);

    auto *files = new QGroupBox(i18n("Files and Notifications"), this);
    auto *filesLayout = new QFormLayout(files);
    filesLayout->setVerticalSpacing(12);
    if (embedMode != Okular::ViewerWidgetMode) {
        auto *watch = new QCheckBox(i18nc("@option:check Config dialog, general page", "Reload document on file change"), this);
        watch->setObjectName(QStringLiteral("kcfg_WatchFile"));
        filesLayout->addRow(watch);
    }
    auto *embedded = new QCheckBox(i18nc("@option:check Config dialog, general page", "Notify about embedded files, forms, or signatures"), this);
    embedded->setObjectName(QStringLiteral("kcfg_ShowEmbeddedContentMessages"));
    filesLayout->addRow(embedded);
    auto *hints = new QCheckBox(i18nc("@option:check Config dialog, general page", "Show hints and info messages"), this);
    hints->setObjectName(QStringLiteral("kcfg_ShowOSD"));
    filesLayout->addRow(hints);
    auto *generators = new QCheckBox(i18nc("@option:check Config dialog, general page", "Show backend selection dialog"), this);
    generators->setObjectName(QStringLiteral("kcfg_ChooseGenerators"));
    filesLayout->addRow(generators);
#if !OKULAR_FORCE_DRM
    if (KAuthorized::authorize(QStringLiteral("skip_drm"))) {
        auto *permissions = new QCheckBox(i18nc("@option:check Config dialog, general page", "Obey DRM limitations"), this);
        permissions->setObjectName(QStringLiteral("kcfg_ObeyDRM"));
        filesLayout->addRow(permissions);
    }
#endif
    layout->addWidget(files);

    if (embedMode != Okular::ViewerWidgetMode) {
        auto *title = new QGroupBox(i18n("Window Title"), this);
        auto *titleLayout = new QFormLayout(title);
        titleLayout->setVerticalSpacing(12);
        auto *showTitle = new QCheckBox(i18nc("@option:check Config dialog, general page", "Display document title in titlebar if available"), this);
        showTitle->setObjectName(QStringLiteral("kcfg_DisplayDocumentTitle"));
        titleLayout->addRow(showTitle);
        auto *nameOrPath = new QComboBox(this);
        nameOrPath->addItem(i18nc("item:inlistbox Config dialog, general page", "Display file name only"));
        nameOrPath->addItem(i18nc("item:inlistbox Config dialog, general page", "Display full file path"));
        nameOrPath->setObjectName(QStringLiteral("kcfg_DisplayDocumentNameOrPath"));
        titleLayout->addRow(i18nc("label:listbox Config dialog, general page", "When not displaying document title:"), nameOrPath);
        layout->addWidget(title);
    }

    if (embedMode != Okular::NativeShellMode) {
        auto *background = new QGroupBox(i18n("Reading Background"), this);
        auto *backgroundLayout = new QHBoxLayout(background);
        auto *enabled = new QCheckBox(i18nc("@label:listbox Config dialog, general page", "Use custom background color:"), this);
        enabled->setObjectName(QStringLiteral("kcfg_UseCustomBackgroundColor"));
        auto *color = new KColorButton(this);
        color->setObjectName(QStringLiteral("kcfg_BackgroundColor"));
        color->setEnabled(false);
        connect(enabled, &QCheckBox::toggled, color, &QWidget::setEnabled);
        backgroundLayout->addWidget(enabled);
        backgroundLayout->addWidget(color);
        layout->addWidget(background);
    }
    layout->addStretch();
}
