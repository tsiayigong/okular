/*
    SPDX-FileCopyrightText: 2026 tsiayigong
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef OKULAR_PAPERMATERIAL_H
#define OKULAR_PAPERMATERIAL_H

#include "appearancepreset.h"
#include <QImage>
#include <QRectF>

class QPainter;

namespace PaperMaterial
{
void paint(QPainter &painter, const QRectF &page, const AppearancePreset &appearance, const QString &imagePath = {});
void compose(QImage &image, const QRect &region, const QSize &pageSize, const AppearancePreset &appearance, const QString &imagePath = {});
}

#endif
