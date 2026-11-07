/*
    SPDX-FileCopyrightText: 2026 tsiayigong
    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "gui/appearancepreset.h"
#include "gui/papermaterial.h"
#include <QPainter>
#include <QTest>

class AppearanceTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void importSchema();
    void croppedMaterialMatchesFullPage();
};

void AppearanceTest::importSchema()
{
    AppearancePreset
        original{QStringLiteral("暖纸"), QColor(QStringLiteral("#99724b")), QColor(QStringLiteral("#f4eee2")), true, 86, 24, QStringLiteral("sand")};
    const auto imported = AppearancePreset::fromJson(original.toJson());
    QVERIFY(imported);
    QCOMPARE(imported->toJson(), original.toJson());
    auto malformed = original.toJson();
    malformed[QStringLiteral("material")] = QStringLiteral("unknown");
    QVERIFY(!AppearancePreset::fromJson(malformed));
    malformed = original.toJson();
    malformed[QStringLiteral("opacity")] = 0;
    QVERIFY(!AppearancePreset::fromJson(malformed));
    malformed = original.toJson();
    malformed.remove(QStringLiteral("blur"));
    QVERIFY(!AppearancePreset::fromJson(malformed));
    malformed = original.toJson();
    malformed[QStringLiteral("paper")] = QStringLiteral("transparent");
    QVERIFY(!AppearancePreset::fromJson(malformed));
}

void AppearanceTest::croppedMaterialMatchesFullPage()
{
    AppearancePreset appearance;
    appearance.acrylic = true;
    appearance.opacity = 60;
    const QSize pageSize(480, 360);
    QImage source(pageSize, QImage::Format_ARGB32_Premultiplied);
    source.fill(appearance.paper);
    QPainter p(&source);
    p.fillRect(QRect(100, 90, 80, 20), Qt::black);
    p.fillRect(QRect(200, 150, 40, 40), Qt::blue);
    p.end();
    const QRect region(70, 60, 240, 180);
    QImage full = source;
    QImage cropped = source.copy(region);
    PaperMaterial::compose(full, source.rect(), pageSize, appearance);
    PaperMaterial::compose(cropped, region, pageSize, appearance);
    QCOMPARE(full.pixelColor(110, 100), QColor(Qt::black));
    QCOMPARE(full.pixelColor(210, 160), QColor(Qt::blue));
    QVERIFY(full.pixelColor(20, 20) != appearance.paper);
    int maxDifference = 0;
    for (int y = 0; y < cropped.height(); ++y) {
        for (int x = 0; x < cropped.width(); ++x) {
            const QColor a = cropped.pixelColor(x, y);
            const QColor b = full.pixelColor(region.x() + x, region.y() + y);
            maxDifference = std::max({maxDifference, qAbs(a.red() - b.red()), qAbs(a.green() - b.green()), qAbs(a.blue() - b.blue())});
        }
    }
    // Qt rounds gradient samples separately for each clipped paint span.
    QVERIFY2(maxDifference <= 1, qPrintable(QStringLiteral("Gradient difference: %1").arg(maxDifference)));
}

QTEST_MAIN(AppearanceTest)
#include "appearancetest.moc"
