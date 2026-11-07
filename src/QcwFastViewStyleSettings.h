// SPDX-FileCopyrightText: 2026 Cheng Lei <kwarph@users.noreply.github.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "QcwDockingGlobal.h"

/**
 * @class QcwFastViewStyleSettings
 * @brief The two style values a fast view popup reads when it opens.
 *
 * @details The popup margin and the border are window-style decisions owned by the host: its
 * style loader writes them before any fast view exists, and QcwFastViewToolBar reads them
 * lazily every time a fast view opens. The store is process-wide because the toolbar can be
 * created after the style has been applied.
 *
 * Values are read and written on the GUI thread only. This is the single global instance:
 * instance() creates it on first use and always returns the same object.
 *
 * @see QcwFastViewToolBar
 */
class QCWDOCK_EXPORT QcwFastViewStyleSettings
{
public:
    /// Returns the process-wide settings object, creating it on first call.
    static QcwFastViewStyleSettings* instance();

    /// Margin kept between the fast view popup and the window edge, in pixels.
    int fastViewMargin() const;

    /// Sets the popup margin; see fastViewMargin().
    void setFastViewMargin(int margin);

    /// Whether the fast view popup draws a frame.
    bool fastViewHasBorder() const;

    /// Sets whether the popup draws a frame; see fastViewHasBorder().
    void setFastViewHasBorder(bool hasBorder);

private:
    int mMargin { 6 };
    bool mBorder { false };
};

/// Shorthand for QcwFastViewStyleSettings::instance().
#ifndef fastViewStyleSettings
#define fastViewStyleSettings QcwFastViewStyleSettings::instance()
#endif
