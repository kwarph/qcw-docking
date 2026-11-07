// SPDX-FileCopyrightText: 2026 Cheng Lei <kwarph@users.noreply.github.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <QtGlobal>

/**
 * @brief Export macro for the QCW docking library.
 *
 * @details Handles symbol visibility for shared libraries.
 * - When building the library (QCWDOCK_LIBRARY defined), symbols are exported.
 * - When using the library, symbols are imported.
 * - For static builds (QCWDOCK_STATIC_LIBRARY defined), the macro is empty.
 *
 * @note This header deliberately carries nothing beyond the macro, so every translation unit
 * can include it without pulling in module-specific vocabulary. Docking and tab-folder types
 * live in QcwDockingTypes.h.
 */
#ifndef QCWDOCK_STATIC_LIBRARY
#ifdef QCWDOCK_LIBRARY
#define QCWDOCK_EXPORT Q_DECL_EXPORT
#else
#define QCWDOCK_EXPORT Q_DECL_IMPORT
#endif
#else
#define QCWDOCK_EXPORT
#endif
