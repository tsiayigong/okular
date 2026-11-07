/*
    SPDX-FileCopyrightText: 2026 tsiayigong
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef OKULAR_DLGREADING_H
#define OKULAR_DLGREADING_H

#include "part.h"
#include <QWidget>

class DlgReading : public QWidget
{
public:
    DlgReading(QWidget *parent, Okular::EmbedMode embedMode);
};

#endif
