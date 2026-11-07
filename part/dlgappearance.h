/*
    SPDX-FileCopyrightText: 2026 tsiayigong
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef OKULAR_DLGAPPEARANCE_H
#define OKULAR_DLGAPPEARANCE_H

#include "gui/appearancepreset.h"
#include <QWidget>

class KColorButton;
class QComboBox;
class QLineEdit;
class QSpinBox;
class AppearancePreview;

class DlgAppearance : public QWidget
{
public:
    explicit DlgAppearance(QWidget *parent, QWidget *accessibility);

private:
    AppearancePreset currentPreset() const;
    void usePreset(const AppearancePreset &preset);
    void updatePreview();
    void showLibrary();
    void createPreset();
    void importPreset();
    void savePreset(const AppearancePreset &preset);
    void selectBackgroundImage();

    QWidget *m_accessibility;
    KColorButton *m_accent;
    KColorButton *m_paper;
    QComboBox *m_material;
    QComboBox *m_background;
    QSpinBox *m_opacity;
    QSpinBox *m_blur;
    QLineEdit *m_name;
    QLineEdit *m_image;
    QLineEdit *m_library;
    AppearancePreview *m_preview;
};

#endif
