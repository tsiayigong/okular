/*
    SPDX-FileCopyrightText: 2026 tsiayigong
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "dlgappearance.h"
#include "gui/papermaterial.h"
#include "settings.h"

#include <KColorButton>
#include <KLocalizedString>
#include <KMessageBox>
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGroupBox>
#include <QImageReader>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPainter>
#include <QPushButton>
#include <QSpinBox>

class AppearancePreview : public QWidget
{
public:
    explicit AppearancePreview(QWidget *parent)
        : QWidget(parent)
    {
        setMinimumSize(280, 180);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        setAccessibleName(i18n("Paper appearance preview"));
    }
    AppearancePreset appearance;
    QString imagePath;

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.fillRect(rect(), QColor(QStringLiteral("#E6EAEE")));
        const QRectF paper = QRectF(rect()).adjusted(18, 18, -18, -18);
        PaperMaterial::paint(painter, paper, appearance, imagePath);
        painter.setPen(QColor(QStringLiteral("#252D38")));
        QFont heading = font();
        heading.setPointSizeF(font().pointSizeF() + 3);
        painter.setFont(heading);
        painter.drawText(paper.adjusted(20, 16, -20, -16), Qt::AlignTop | Qt::AlignLeft, i18n("A comfortable place to read"));
        painter.fillRect(QRectF(paper.left() + 20, paper.top() + 50, 84, 3), appearance.accent);
        painter.setFont(font());
        painter.setPen(QColor(QStringLiteral("#596372")));
        painter.drawText(paper.adjusted(20, 68, -20, -12), Qt::TextWordWrap, i18n("Text, diagrams and ideas.\nPingFang SC · Regular"));
    }
};

namespace
{
const QStringList backgrounds{QStringLiteral("mist"), QStringLiteral("sand"), QStringLiteral("ice")};
QList<AppearancePreset> builtinPresets()
{
    return {{i18n("Mist"), QColor(QStringLiteral("#0A84FF")), QColor(QStringLiteral("#FAFAF8")), false, 82, 24, QStringLiteral("mist")},
            {i18n("Warm paper"), QColor(QStringLiteral("#99724B")), QColor(QStringLiteral("#F4EEE2")), false, 86, 24, QStringLiteral("sand")},
            {i18n("Glacier"), QColor(QStringLiteral("#287BAD")), QColor(QStringLiteral("#EAF3F7")), true, 78, 28, QStringLiteral("ice")}};
}
QLineEdit *hiddenSetting(QWidget *parent, const QString &key)
{
    auto *value = new QLineEdit(parent);
    value->setObjectName(QStringLiteral("kcfg_") + key);
    value->setMaxLength(1024 * 1024);
    value->hide();
    return value;
}
}

DlgAppearance::DlgAppearance(QWidget *parent, QWidget *accessibility)
    : QWidget(parent)
    , m_accessibility(accessibility)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 12, 16, 12);
    layout->setSpacing(16);
    auto *summary = new QHBoxLayout;
    m_preview = new AppearancePreview(this);
    m_preview->setMaximumHeight(200);
    summary->addWidget(m_preview, 1);
    auto *actions = new QVBoxLayout;
    auto *library = new QPushButton(i18n("Manage Appearances…"), this);
    library->setObjectName(QStringLiteral("manageAppearances"));
    auto *create = new QPushButton(i18n("New Appearance…"), this);
    create->setObjectName(QStringLiteral("createAppearance"));
    auto *import = new QPushButton(i18n("Import Appearance…"), this);
    import->setObjectName(QStringLiteral("importAppearance"));
    actions->addWidget(library);
    actions->addWidget(create);
    actions->addWidget(import);
    actions->addStretch();
    summary->addLayout(actions);
    layout->addLayout(summary);

    auto *interfaceGroup = new QGroupBox(i18n("Interface"), this);
    auto *interfaceForm = new QFormLayout(interfaceGroup);
    auto *theme = new QComboBox(this);
    theme->setObjectName(QStringLiteral("kcfg_InterfaceTheme"));
    theme->addItems({i18n("Follow system"), i18n("Light"), i18n("Dark")});
    interfaceForm->addRow(i18n("Appearance:"), theme);
    interfaceForm->addRow(i18n("Interface font:"), new QLabel(i18n("PingFang SC · Regular"), this));
    m_accent = new KColorButton(this);
    m_accent->setObjectName(QStringLiteral("kcfg_AccentColor"));
    m_accent->setFixedWidth(64);
    interfaceForm->addRow(i18n("Accent color:"), m_accent);
    auto *autoHide = new QCheckBox(i18n("Automatically hide the main toolbar"), this);
    autoHide->setObjectName(QStringLiteral("kcfg_AutoHideMainToolBar"));
    interfaceForm->addRow(autoHide);
    auto *hint = new QLabel(i18n("Move the pointer to the top edge of the document to show the toolbar. Press Ctrl+Alt+T to focus it."), this);
    hint->setWordWrap(true);
    interfaceForm->addRow(hint);
    auto *backgroundRow = new QHBoxLayout;
    auto *useBackground = new QCheckBox(i18nc("@label:listbox Config dialog, general page", "Use custom background color:"), this);
    useBackground->setObjectName(QStringLiteral("kcfg_UseCustomBackgroundColor"));
    auto *backgroundColor = new KColorButton(this);
    backgroundColor->setObjectName(QStringLiteral("kcfg_BackgroundColor"));
    backgroundColor->setFixedWidth(64);
    backgroundColor->setEnabled(false);
    connect(useBackground, &QCheckBox::toggled, backgroundColor, &QWidget::setEnabled);
    backgroundRow->addWidget(useBackground);
    backgroundRow->addStretch();
    backgroundRow->addWidget(backgroundColor);
    interfaceForm->addRow(backgroundRow);
    layout->addWidget(interfaceGroup);

    auto *paperGroup = new QGroupBox(i18n("Paper Background"), this);
    auto *paperForm = new QFormLayout(paperGroup);
    m_paper = new KColorButton(this);
    m_paper->setObjectName(QStringLiteral("appearancePaperColor"));
    m_paper->setFixedWidth(64);
    auto *sharedPaper = accessibility->findChild<KColorButton *>(QStringLiteral("kcfg_PaperColor"));
    m_paper->setColor(sharedPaper->color());
    connect(m_paper, &KColorButton::changed, sharedPaper, &KColorButton::setColor);
    connect(sharedPaper, &KColorButton::changed, m_paper, &KColorButton::setColor);
    paperForm->addRow(i18n("Paper color:"), m_paper);
    m_material = new QComboBox(this);
    m_material->setObjectName(QStringLiteral("kcfg_PaperMaterial"));
    m_material->addItems({i18n("Solid color"), i18n("Acrylic")});
    paperForm->addRow(i18n("Material:"), m_material);
    m_background = new QComboBox(this);
    m_background->setObjectName(QStringLiteral("kcfg_PaperBackground"));
    m_background->addItems({i18n("Mist"), i18n("Dunes"), i18n("Glacier")});
    paperForm->addRow(i18n("Background:"), m_background);
    m_opacity = new QSpinBox(this);
    m_opacity->setObjectName(QStringLiteral("kcfg_PaperOpacity"));
    m_opacity->setRange(45, 100);
    m_opacity->setSuffix(QStringLiteral("%"));
    paperForm->addRow(i18n("Opacity:"), m_opacity);
    m_blur = new QSpinBox(this);
    m_blur->setObjectName(QStringLiteral("kcfg_PaperBlur"));
    m_blur->setRange(8, 48);
    m_blur->setSuffix(i18n(" px"));
    paperForm->addRow(i18n("Blur:"), m_blur);
    auto *imageActions = new QHBoxLayout;
    auto *image = new QPushButton(i18n("Choose Background Image…"), this);
    auto *clearImage = new QPushButton(i18n("Use Preset Background"), this);
    imageActions->addWidget(image);
    imageActions->addWidget(clearImage);
    paperForm->addRow(imageActions);
    layout->addWidget(paperGroup);
    layout->addStretch();

    m_name = hiddenSetting(this, QStringLiteral("AppearanceName"));
    m_image = hiddenSetting(this, QStringLiteral("PaperBackgroundImage"));
    m_library = hiddenSetting(this, QStringLiteral("CustomAppearances"));
    connect(m_accent, &KColorButton::changed, this, &DlgAppearance::updatePreview);
    connect(m_paper, &KColorButton::changed, this, &DlgAppearance::updatePreview);
    connect(m_background, &QComboBox::currentIndexChanged, this, &DlgAppearance::updatePreview);
    connect(m_opacity, &QSpinBox::valueChanged, this, &DlgAppearance::updatePreview);
    connect(m_blur, &QSpinBox::valueChanged, this, &DlgAppearance::updatePreview);
    connect(m_image, &QLineEdit::textChanged, this, &DlgAppearance::updatePreview);
    connect(m_material, &QComboBox::currentIndexChanged, this, [this](int index) {
        m_background->setEnabled(index == 1);
        m_blur->setEnabled(index == 1);
        m_opacity->setEnabled(index == 1);
        updatePreview();
    });
    connect(library, &QPushButton::clicked, this, &DlgAppearance::showLibrary);
    connect(create, &QPushButton::clicked, this, &DlgAppearance::createPreset);
    connect(import, &QPushButton::clicked, this, &DlgAppearance::importPreset);
    connect(image, &QPushButton::clicked, this, &DlgAppearance::selectBackgroundImage);
    connect(clearImage, &QPushButton::clicked, m_image, &QLineEdit::clear);
    m_background->setEnabled(false);
    m_opacity->setEnabled(false);
    m_blur->setEnabled(false);
}

AppearancePreset DlgAppearance::currentPreset() const
{
    return {m_name->text(),
            m_accent->color(),
            m_paper->color(),
            m_material->currentIndex() == 1,
            m_opacity->value(),
            m_blur->value(),
            backgrounds.at(m_background->currentIndex())};
}
void DlgAppearance::usePreset(const AppearancePreset &preset)
{
    m_name->setText(preset.name);
    m_accent->setColor(preset.accent);
    m_paper->setColor(preset.paper);
    m_material->setCurrentIndex(preset.acrylic ? 1 : 0);
    m_opacity->setValue(preset.opacity);
    m_blur->setValue(preset.blur);
    m_background->setCurrentIndex(backgrounds.indexOf(preset.background));
    m_image->clear();
    m_accessibility->findChild<QCheckBox *>(QStringLiteral("kcfg_ChangeColors"))->setChecked(true);
    m_accessibility->findChild<QComboBox *>(QStringLiteral("kcfg_RenderMode"))->setCurrentIndex(Okular::Settings::EnumRenderMode::Paper);
    updatePreview();
}
void DlgAppearance::updatePreview()
{
    m_preview->appearance = currentPreset();
    m_preview->imagePath = m_image->text();
    m_preview->update();
}
void DlgAppearance::showLibrary()
{
    QList<AppearancePreset> presets = builtinPresets();
    QJsonParseError error;
    const QJsonDocument custom = QJsonDocument::fromJson(m_library->text().toUtf8(), &error);
    if (error.error != QJsonParseError::NoError || !custom.isArray()) {
        KMessageBox::error(this, i18n("The saved appearance library could not be read."));
        return;
    }
    for (const QJsonValue &value : custom.array()) {
        const auto preset = AppearancePreset::fromJson(value.toObject());
        if (!preset) {
            KMessageBox::error(this, i18n("The saved appearance library contains an invalid appearance."));
            return;
        }
        presets.append(*preset);
    }
    QDialog dialog(this);
    dialog.setWindowTitle(i18n("Appearance Library"));
    dialog.resize(660, 340);
    auto *layout = new QVBoxLayout(&dialog);
    auto *body = new QHBoxLayout;
    auto *list = new QListWidget(&dialog);
    list->setObjectName(QStringLiteral("appearancePresetList"));
    list->setMaximumWidth(180);
    for (const auto &preset : std::as_const(presets)) {
        list->addItem(preset.name);
    }
    auto *preview = new AppearancePreview(&dialog);
    body->addWidget(list);
    body->addWidget(preview, 1);
    layout->addLayout(body, 1);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok, &dialog);
    buttons->button(QDialogButtonBox::Ok)->setText(i18n("Use Appearance"));
    layout->addWidget(buttons);
    connect(list, &QListWidget::currentRowChanged, &dialog, [preview, &presets](int row) {
        if (row >= 0) {
            preview->appearance = presets.at(row);
            preview->update();
        }
    });
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    list->setCurrentRow(0);
    if (dialog.exec() == QDialog::Accepted) {
        usePreset(presets.at(list->currentRow()));
    }
}
void DlgAppearance::createPreset()
{
    AppearancePreset draft = currentPreset();
    draft.name = i18n("My appearance");
    QDialog dialog(this);
    dialog.setWindowTitle(i18n("New Appearance"));
    dialog.resize(740, 370);
    auto *layout = new QVBoxLayout(&dialog);
    auto *body = new QHBoxLayout;
    auto *form = new QFormLayout;
    auto *name = new QLineEdit(draft.name, &dialog);
    name->setObjectName(QStringLiteral("appearanceName"));
    name->setMaxLength(40);
    form->addRow(i18n("Name:"), name);
    auto *accent = new KColorButton(draft.accent, &dialog);
    form->addRow(i18n("Accent color:"), accent);
    auto *paper = new KColorButton(draft.paper, &dialog);
    form->addRow(i18n("Paper color:"), paper);
    auto *material = new QComboBox(&dialog);
    material->addItems({i18n("Solid color"), i18n("Acrylic")});
    material->setCurrentIndex(draft.acrylic ? 1 : 0);
    form->addRow(i18n("Material:"), material);
    auto *background = new QComboBox(&dialog);
    background->addItems({i18n("Mist"), i18n("Dunes"), i18n("Glacier")});
    background->setCurrentIndex(backgrounds.indexOf(draft.background));
    form->addRow(i18n("Background:"), background);
    auto *opacity = new QSpinBox(&dialog);
    opacity->setRange(45, 100);
    opacity->setSuffix(QStringLiteral("%"));
    opacity->setValue(draft.opacity);
    form->addRow(i18n("Opacity:"), opacity);
    auto *blur = new QSpinBox(&dialog);
    blur->setRange(8, 48);
    blur->setSuffix(i18n(" px"));
    blur->setValue(draft.blur);
    form->addRow(i18n("Blur:"), blur);
    auto *preview = new AppearancePreview(&dialog);
    preview->appearance = draft;
    body->addLayout(form);
    body->addWidget(preview, 1);
    layout->addLayout(body);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok, &dialog);
    buttons->button(QDialogButtonBox::Ok)->setText(i18n("Create and Use"));
    layout->addWidget(buttons);
    auto update = [&] {
        draft = {name->text().trimmed(),
                 accent->color(),
                 paper->color(),
                 material->currentIndex() == 1,
                 opacity->value(),
                 blur->value(),
                 backgrounds.at(background->currentIndex())};
        preview->appearance = draft;
        preview->update();
        opacity->setEnabled(draft.acrylic);
        blur->setEnabled(draft.acrylic);
        background->setEnabled(draft.acrylic);
        buttons->button(QDialogButtonBox::Ok)->setEnabled(!draft.name.isEmpty());
    };
    connect(name, &QLineEdit::textChanged, &dialog, update);
    connect(accent, &KColorButton::changed, &dialog, update);
    connect(paper, &KColorButton::changed, &dialog, update);
    connect(material, &QComboBox::currentIndexChanged, &dialog, update);
    connect(background, &QComboBox::currentIndexChanged, &dialog, update);
    connect(opacity, &QSpinBox::valueChanged, &dialog, update);
    connect(blur, &QSpinBox::valueChanged, &dialog, update);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    update();
    if (dialog.exec() == QDialog::Accepted) {
        savePreset(draft);
    }
}
void DlgAppearance::savePreset(const AppearancePreset &preset)
{
    QJsonParseError error;
    const auto document = QJsonDocument::fromJson(m_library->text().toUtf8(), &error);
    if (error.error != QJsonParseError::NoError || !document.isArray()) {
        KMessageBox::error(this, i18n("The saved appearance library could not be read."));
        return;
    }
    QJsonArray library = document.array();
    library.append(preset.toJson());
    m_library->setText(QString::fromUtf8(QJsonDocument(library).toJson(QJsonDocument::Compact)));
    usePreset(preset);
}
void DlgAppearance::importPreset()
{
    const QString path = QFileDialog::getOpenFileName(this, i18n("Import Appearance"), {}, i18n("Okular Appearance (*.json)"));
    if (path.isEmpty()) {
        return;
    }
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        KMessageBox::error(this, file.errorString());
        return;
    }
    if (file.size() > 64000) {
        KMessageBox::error(this, i18n("Appearance files must be smaller than 64 KB."));
        return;
    }
    const auto preset = AppearancePreset::fromJson(QJsonDocument::fromJson(file.readAll()).object());
    if (!preset) {
        KMessageBox::error(this, i18n("Choose a valid Okular appearance file (version 1)."));
        return;
    }
    QDialog dialog(this);
    dialog.setWindowTitle(i18n("Import Appearance"));
    auto *layout = new QVBoxLayout(&dialog);
    auto *preview = new AppearancePreview(&dialog);
    preview->appearance = *preset;
    layout->addWidget(preview);
    layout->addWidget(new QLabel(preset->name, &dialog));
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok, &dialog);
    buttons->button(QDialogButtonBox::Ok)->setText(i18n("Import and Use"));
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    if (dialog.exec() == QDialog::Accepted) {
        savePreset(*preset);
    }
}
void DlgAppearance::selectBackgroundImage()
{
    const QString path = QFileDialog::getOpenFileName(this, i18n("Choose Background Image"), {}, i18n("Images (*.png *.jpg *.jpeg *.webp)"));
    if (path.isEmpty()) {
        return;
    }
    QImageReader reader(path);
    const auto format = reader.format().toLower();
    if (QFileInfo(path).size() > 2 * 1024 * 1024 || (format != "png" && format != "jpeg" && format != "webp")) {
        KMessageBox::error(this, i18n("Choose a PNG, JPEG or WebP image smaller than 2 MB."));
        return;
    }
    reader.setScaledSize(reader.size().scaled(QSize(1920, 1080), Qt::KeepAspectRatio));
    if (reader.read().isNull()) {
        KMessageBox::error(this, reader.errorString());
        return;
    }
    m_image->setText(path);
    m_material->setCurrentIndex(1);
}
