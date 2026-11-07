/*
    SPDX-FileCopyrightText: 2026 tsiayigong
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "papermaterial.h"

#include <QDateTime>
#include <QFileInfo>
#include <QGraphicsBlurEffect>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QImageReader>
#include <QPainter>
#include <QPixmapCache>

namespace
{
QPixmap blurredImage(const QString &path, int radius)
{
    const QString key = QStringLiteral("okular-paper:%1:%2:%3").arg(path).arg(QFileInfo(path).lastModified().toMSecsSinceEpoch()).arg(radius);
    QPixmap texture;
    if (QPixmapCache::find(key, &texture)) {
        return texture;
    }
    QImageReader reader(path);
    reader.setAutoTransform(true);
    reader.setScaledSize(reader.size().scaled(QSize(1920, 1080), Qt::KeepAspectRatio));
    const QImage image = reader.read();
    if (image.isNull()) {
        qWarning() << "Cannot read paper background" << path << reader.errorString();
        return {};
    }
    QGraphicsScene scene;
    auto *item = scene.addPixmap(QPixmap::fromImage(image));
    auto *effect = new QGraphicsBlurEffect;
    effect->setBlurRadius(radius);
    effect->setBlurHints(QGraphicsBlurEffect::QualityHint);
    item->setGraphicsEffect(effect);
    scene.setSceneRect(image.rect());
    texture = QPixmap(image.size());
    texture.fill(Qt::transparent);
    QPainter painter(&texture);
    scene.render(&painter, image.rect(), image.rect());
    painter.end();
    QPixmapCache::insert(key, texture);
    return texture;
}
}

void PaperMaterial::paint(QPainter &painter, const QRectF &page, const AppearancePreset &appearance, const QString &imagePath)
{
    if (!appearance.acrylic) {
        painter.fillRect(page, appearance.paper);
        return;
    }
    painter.save();
    painter.setClipRect(page, Qt::IntersectClip);
    const bool sand = appearance.background == QLatin1String("sand");
    const bool ice = appearance.background == QLatin1String("ice");
    QLinearGradient base(page.topLeft(), page.bottomRight());
    base.setColorAt(0, QColor(sand ? "#E8DFD0" : ice ? "#D7E8E8" : "#D2DCE3"));
    base.setColorAt(1, QColor(sand ? "#D7DCDB" : ice ? "#BDCFDF" : "#DFE4E2"));
    painter.fillRect(page, base);
    QRadialGradient cloud(page.topLeft() + QPointF(page.width() * 0.15, page.height() * 0.2), page.width() * (0.5 + appearance.blur / 100.0));
    cloud.setColorAt(0, QColor(sand ? "#D6CBB9" : ice ? "#A9CAD3" : "#B7CBD9"));
    cloud.setColorAt(1, Qt::transparent);
    painter.fillRect(page, cloud);
    QRadialGradient haze(page.topLeft() + QPointF(page.width() * 0.9, page.height() * 0.85), page.width() * 0.6);
    haze.setColorAt(0, QColor(sand ? "#BDBFB5" : ice ? "#C1C9E3" : "#C9C5D9"));
    haze.setColorAt(1, Qt::transparent);
    painter.fillRect(page, haze);
    if (!imagePath.isEmpty()) {
        const QPixmap texture = blurredImage(imagePath, appearance.blur);
        if (!texture.isNull()) {
            const QSizeF size = QSizeF(texture.size()).scaled(page.size(), Qt::KeepAspectRatioByExpanding);
            const QRectF target(page.center() - QPointF(size.width() / 2, size.height() / 2), size);
            painter.drawPixmap(target, texture, texture.rect());
        }
    }
    QColor tint = appearance.paper;
    tint.setAlphaF(appearance.opacity / 100.0);
    painter.fillRect(page, tint);
    painter.restore();
}

void PaperMaterial::compose(QImage &image, const QRect &region, const QSize &pageSize, const AppearancePreset &appearance, const QString &imagePath)
{
    Q_ASSERT(image.format() == QImage::Format_ARGB32_Premultiplied);
    Q_ASSERT(image.size() == region.size());
    QImage material(image.size(), QImage::Format_ARGB32_Premultiplied);
    material.fill(appearance.paper);
    QPainter painter(&material);
    painter.translate(-region.topLeft());
    paint(painter, QRect(QPoint(), pageSize), appearance, imagePath);
    painter.end();
    const QRgb paper = appearance.paper.rgb();
    for (int y = 0; y < image.height(); ++y) {
        auto *pixels = reinterpret_cast<QRgb *>(image.scanLine(y));
        const auto *background = reinterpret_cast<const QRgb *>(material.constScanLine(y));
        for (int x = 0; x < image.width(); ++x) {
            if (pixels[x] == paper) {
                pixels[x] = background[x];
            }
        }
    }
}
