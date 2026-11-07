/*
    SPDX-FileCopyrightText: 2026 tsiayigong
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef OKULAR_DESKTOPAPPEARANCE_H
#define OKULAR_DESKTOPAPPEARANCE_H

#include <QColor>
#include <QObject>

class DesktopAppearance : public QObject
{
public:
    static DesktopAppearance *instance();
    void setTheme(int theme, const QColor &accent);

private:
    explicit DesktopAppearance(QObject *parent);
    void apply();
    int m_theme = -1;
    QColor m_accent;
};

#endif
