/*
    SPDX-FileCopyrightText: 2006 Pino Toscano <toscano.pino@tiscali.it>
    SPDX-FileCopyrightText: 2026 tsiayigong
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "dlgreading.h"

#include <KLocalization>
#include <KLocalizedString>
#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QGuiApplication>
#include <QSpinBox>
#include <QVBoxLayout>

DlgReading::DlgReading(QWidget *parent, Okular::EmbedMode embedMode)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 8, 24, 16);
    layout->setSpacing(16);

    auto *pages = new QGroupBox(i18n("Pages and Zoom"), this);
    auto *pageLayout = new QFormLayout(pages);
    pageLayout->setVerticalSpacing(12);
    auto *zoomLayout = new QHBoxLayout;
    auto *customZoom = new QSpinBox(this);
    KLocalization::setupSpinBoxFormatString(customZoom, ki18nc("@item:inlistbox Config dialog, general page, custom zoom factor", "%v%"));
    customZoom->setObjectName(QStringLiteral("kcfg_CustomZoomFactor"));
    customZoom->hide();
    auto *defaultZoom = new QComboBox(this);
    defaultZoom->addItem(i18nc("@item:inlistbox Config dialog, general page, default zoom", "100%"));
    defaultZoom->addItem(i18nc("@item:inlistbox Config dialog, general page, default zoom", "Fit Width"));
    defaultZoom->addItem(i18nc("@item:inlistbox Config dialog, general page, default zoom", "Fit Page"));
    defaultZoom->addItem(i18nc("@item:inlistbox Config dialog, general page, default zoom", "Auto Fit"));
    defaultZoom->addItem(i18nc("@item:inlistbox Config dialog, general page, default zoom", "Custom"));
    defaultZoom->setToolTip(i18nc("@item:inlistbox Config dialog, general page, default zoom", "Defines the default zoom mode for files which were never opened before. For files which were opened before the previous zoom is applied."));
    defaultZoom->setObjectName(QStringLiteral("kcfg_ZoomMode"));
    zoomLayout->addWidget(defaultZoom);
    zoomLayout->addWidget(customZoom);
    pageLayout->addRow(i18nc("label:listbox Config dialog, general page, default zoom", "Default zoom:"), zoomLayout);
    connect(defaultZoom, &QComboBox::currentIndexChanged, customZoom, [customZoom](int index) { customZoom->setVisible(index == 4); });

    auto *useFileInfo = new QCheckBox(i18nc("@option:check Config dialog, general page", "Use file information to determine whether to open in continuous mode by default"), this);
    useFileInfo->setObjectName(QStringLiteral("kcfg_UseFileInfoForViewContinuous"));
    pageLayout->addRow(useFileInfo);
    auto *continuous = new QCheckBox(this);
    continuous->setObjectName(QStringLiteral("kcfg_ViewContinuous"));
    pageLayout->addRow(continuous);
    const auto updateContinuousText = [useFileInfo, continuous] {
        continuous->setText(useFileInfo->isChecked()
                                ? i18nc("@option:check Config dialog, general page", "For files without file information, open in continuous mode by default")
                                : i18nc("@option:check Config dialog, general page", "Open in continuous mode by default"));
    };
    connect(useFileInfo, &QCheckBox::toggled, this, updateContinuousText);
    updateContinuousText();

    if (embedMode != Okular::ViewerWidgetMode) {
        auto *rtl = new QCheckBox(i18nc("@option:check Config dialog, general page", "Right to left reading direction"), this);
        rtl->setObjectName(QStringLiteral("kcfg_rtlReadingDirection"));
        pageLayout->addRow(rtl);
    }
    auto *columns = new QSpinBox(this);
    columns->setRange(3, 10);
    columns->setObjectName(QStringLiteral("kcfg_ViewColumns"));
    pageLayout->addRow(i18nc("@label:spinbox Config dialog, general page", "Overview columns:"), columns);
    layout->addWidget(pages);

    auto *scrolling = new QGroupBox(i18n("Scrolling and Navigation"), this);
    auto *scrollLayout = new QFormLayout(scrolling);
    scrollLayout->setVerticalSpacing(12);
    auto *smooth = new QCheckBox(i18nc("@option:check Config dialog, general page", "Use smooth scrolling"), this);
    smooth->setObjectName(QStringLiteral("kcfg_SmoothScrolling"));
    scrollLayout->addRow(smooth);
    auto *scrollbars = new QCheckBox(i18nc("@option:check Config dialog, general page", "Show scrollbars"), this);
    scrollbars->setObjectName(QStringLiteral("kcfg_ShowScrollBars"));
    scrollLayout->addRow(scrollbars);
    if (embedMode != Okular::ViewerWidgetMode) {
        auto *thumbnails = new QCheckBox(i18nc("@option:check Config dialog, general page", "Link the thumbnails with the page"), this);
        thumbnails->setObjectName(QStringLiteral("kcfg_SyncThumbnailsViewport"));
        scrollLayout->addRow(thumbnails);
    }
    auto *overlap = new QSpinBox(this);
    overlap->setRange(0, 50);
    overlap->setSingleStep(5);
    KLocalization::setupSpinBoxFormatString(overlap, ki18nc("@item:spinbox Config dialog, general page, page overlap factor", "%v%"));
    overlap->setToolTip(i18nc("@info:tooltip Config dialog, general page", "Defines how much of the current viewing area will still be visible when pressing the Page Up/Down keys."));
    overlap->setObjectName(QStringLiteral("kcfg_ScrollOverlap"));
    scrollLayout->addRow(i18nc("@label:spinbox Config dialog, general page", "Page Up/Down overlap:"), overlap);
    if (QGuiApplication::platformName() != QLatin1String("wayland")) {
        auto *wrap = new QCheckBox(i18nc("@option:check Config dialog, general page", "When using Browse Tool, wrap cursor at screen edges"), this);
        wrap->setObjectName(QStringLiteral("kcfg_DragBeyondScreenEdges"));
        scrollLayout->addRow(wrap);
    }
    layout->addWidget(scrolling);
    layout->addStretch();
}
