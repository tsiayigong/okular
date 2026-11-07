/*
    SPDX-FileCopyrightText: 2026 tsiayigong
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "appearancepreset.h"
#include <QRegularExpression>

QJsonObject AppearancePreset::toJson() const
{
    return {{QStringLiteral("format"), QStringLiteral("okular-appearance")},
            {QStringLiteral("version"), 1},
            {QStringLiteral("name"), name},
            {QStringLiteral("accent"), accent.name()},
            {QStringLiteral("paper"), paper.name()},
            {QStringLiteral("material"), acrylic ? QStringLiteral("acrylic") : QStringLiteral("solid")},
            {QStringLiteral("opacity"), opacity},
            {QStringLiteral("blur"), blur},
            {QStringLiteral("background"), background}};
}

std::optional<AppearancePreset> AppearancePreset::fromJson(const QJsonObject &object)
{
    static const QRegularExpression color(QStringLiteral("^#[0-9a-fA-F]{6}$"));
    const QString name = object.value(QStringLiteral("name")).toString().trimmed();
    const QString accent = object.value(QStringLiteral("accent")).toString();
    const QString paper = object.value(QStringLiteral("paper")).toString();
    const QString material = object.value(QStringLiteral("material")).toString();
    const QString background = object.value(QStringLiteral("background")).toString();
    const double opacity = object.value(QStringLiteral("opacity")).toDouble(-1);
    const double blur = object.value(QStringLiteral("blur")).toDouble(-1);
    if (object.value(QStringLiteral("format")).toString() != QLatin1String("okular-appearance") || object.value(QStringLiteral("version")).toInt() != 1
        || name.isEmpty() || name.size() > 40 || !color.match(accent).hasMatch() || !color.match(paper).hasMatch()
        || (material != QLatin1String("solid") && material != QLatin1String("acrylic"))
        || (background != QLatin1String("mist") && background != QLatin1String("sand") && background != QLatin1String("ice")) || opacity < 45 || opacity > 100
        || blur < 8 || blur > 48) {
        return std::nullopt;
    }
    return AppearancePreset{name, QColor(accent), QColor(paper), material == QLatin1String("acrylic"), qRound(opacity), qRound(blur), background};
}
