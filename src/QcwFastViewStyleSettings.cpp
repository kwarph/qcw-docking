// SPDX-FileCopyrightText: 2026 Cheng Lei <kwarph@users.noreply.github.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "QcwFastViewStyleSettings.h"

#include <QGlobalStatic>

Q_GLOBAL_STATIC(QcwFastViewStyleSettings, fastViewStyleSettingsInstance)

QcwFastViewStyleSettings* QcwFastViewStyleSettings::instance()
{
    return fastViewStyleSettingsInstance;
}

int QcwFastViewStyleSettings::fastViewMargin() const
{
    return mMargin;
}

void QcwFastViewStyleSettings::setFastViewMargin(int margin)
{
    mMargin = margin;
}

bool QcwFastViewStyleSettings::fastViewHasBorder() const
{
    return mBorder;
}

void QcwFastViewStyleSettings::setFastViewHasBorder(bool hasBorder)
{
    mBorder = hasBorder;
}
