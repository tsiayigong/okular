/*
    SPDX-FileCopyrightText: 2026 tsiayigong
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef OKULAR_WINDOWMATERIAL_H
#define OKULAR_WINDOWMATERIAL_H

#include <QObject>

class QWidget;

class WindowMaterial : public QObject
{
public:
    explicit WindowMaterial(QObject *parent);

protected:
    bool eventFilter(QObject *object, QEvent *event) override;

private:
    void apply(QWidget *widget);
};

#endif
