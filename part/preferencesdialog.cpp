/*
    SPDX-FileCopyrightText: 2004 Enrico Ros <eros.kde@email.it>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

// reimplementing this
#include "preferencesdialog.h"

#include <KLocalizedString>
#include <KTitleWidget>

// single config pages
#include "dlgaccessibility.h"
#include "dlgannotations.h"
#include "dlgappearance.h"
#include "dlgdebug.h"
#include "dlgeditor.h"
#include "dlggeneral.h"
#include "dlgperformance.h"
#include "dlgpresentation.h"
#include "dlgreading.h"

#include <QLabel>
#include <QPushButton>
#include <QScreen>

PreferencesDialog::PreferencesDialog(QWidget *parent, KConfigSkeleton *skeleton, Okular::EmbedMode embedMode, const QString &editCmd)
    : KConfigDialog(parent, QStringLiteral("preferences"), skeleton)
{
    setWindowModality(Qt::ApplicationModal);
    setWindowTitle(i18n("Settings"));
    setFaceType(KPageDialog::FlatList);
    resize(QSize(980, 740).boundedTo(screen()->availableGeometry().size() * 0.9));
    // KConfigDialog connects its existing buttons to the settings managers.
    delete button(QDialogButtonBox::Help);
    button(QDialogButtonBox::Ok)->setText(i18n("Done"));
    button(QDialogButtonBox::Ok)->setDefault(true);

    auto *heading = qobject_cast<KTitleWidget *>(pageWidget()->pageHeader());
    heading->setContentsMargins(24, 16, 24, 12);
    const auto updateHeading = [this, heading] {
        auto *page = currentPage();
        heading->setText(page ? QStringLiteral("<span style=\"font-size:22px;font-weight:600\">%1</span>").arg(page->name().toHtmlEscaped()) : QString());
        heading->setComment(page ? page->header() : QString());
    };
    connect(this, &KPageDialog::currentPageChanged, this, updateHeading);
    connect(pageWidget()->model(), &QAbstractItemModel::dataChanged, this, updateHeading);

    m_general = new DlgGeneral(this, embedMode);
    m_performance = new DlgPerformance(this);
    m_accessibility = new DlgAccessibility(this);
    m_accessibilityPage = nullptr;
    m_presentation = nullptr;
    m_annotations = nullptr;
    m_annotationsPage = nullptr;
    m_editor = nullptr;
    m_signatures = nullptr;
#ifdef OKULAR_DEBUG_CONFIGPAGE
    m_debug = new DlgDebug(this);
#endif

    addPage(m_general, i18n("General"), QStringLiteral("preferences-system"), i18n("General Options"));
    if (embedMode == Okular::NativeShellMode) {
        addPage(new DlgAppearance(this, m_accessibility),
                i18n("Appearance"),
                QStringLiteral("preferences-desktop-theme"),
                i18n("Appearance and Reading Space"));
    }
    addPage(new DlgReading(this, embedMode), i18n("Reading"), QStringLiteral("accessories-ebook-reader"), i18n("Page layout and navigation"));
    m_accessibilityPage =
        addPage(m_accessibility, i18n("Accessibility"), QStringLiteral("preferences-desktop-accessibility"), i18n("Accessibility Reading Aids"));
    addPage(m_performance, i18n("Performance"), QStringLiteral("preferences-system-performance"), i18n("Performance Tuning"));
    if (embedMode == Okular::ViewerWidgetMode) {
        setWindowTitle(i18n("Configure Viewer"));
    } else {
        m_presentation = new DlgPresentation(this);
        m_annotations = new DlgAnnotations(this);
        addPage(m_presentation, i18n("Presentation"), QStringLiteral("view-presentation"), i18n("Options for Presentation Mode"));
        m_annotationsPage = addPage(m_annotations, i18n("Annotations"), QStringLiteral("draw-freehand"), i18n("Annotation Options"));
        if (editCmd.isEmpty()) {
            m_editor = new DlgEditor(this);
            addPage(m_editor, i18n("Editor"), QStringLiteral("accessories-text-editor"), i18n("Editor Options"));
        } else {
            QString editStr =
                i18nc("Give the user a hint, that it enabled the option --editor-cmd together with the current value of the option.",
                      "The editor was set by the command line to \n %1 \nIf you want to use the setting, start okular without the option --editor-cmd",
                      editCmd);
            auto editor = new QLabel(editStr, this);
            addPage(editor, i18n("Editor"), QStringLiteral("accessories-text-editor"), i18n("Editor Options"));
        }
    }
#ifdef OKULAR_DEBUG_CONFIGPAGE
    addPage(m_debug, "Debug", "system-run", "Debug options");
#endif
    setHelp(QStringLiteral("configure"), QStringLiteral("okular"));
}

void PreferencesDialog::switchToAccessibilityPage()
{
    if (m_accessibilityPage) {
        setCurrentPage(m_accessibilityPage);
    }
}

void PreferencesDialog::switchToAnnotationsPage()
{
    if (m_annotationsPage) {
        setCurrentPage(m_annotationsPage);
    }
}
