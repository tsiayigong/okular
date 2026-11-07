/*
    SPDX-FileCopyrightText: 2026 tsiayigong
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef OKULAR_APPEARANCEPRESET_H
#define OKULAR_APPEARANCEPRESET_H

#include <QColor>
#include <QJsonObject>
#include <QString>
#include <optional>

struct AppearancePreset {
    QString name;
    QColor accent = QColor(QStringLiteral("#0A84FF"));
    QColor paper = QColor(QStringLiteral("#FAFAF8"));
    bool acrylic = false;
    int opacity = 82;
    int blur = 24;
    QString background = QStringLiteral("mist");

    QJsonObject toJson() const;
    static std::optional<AppearancePreset> fromJson(const QJsonObject &object);
};

#endif
