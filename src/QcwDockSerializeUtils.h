// SPDX-FileCopyrightText: 2026 Cheng Lei <kwarph@users.noreply.github.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <QByteArray>
#include <QBuffer>
#include <QDataStream>
#include <QIODevice>

QT_BEGIN_NAMESPACE

/**
 * @struct QcwDockSerializeUtils
 * @brief Stateless helpers for the QDataStream round trip the folder widgets store as bytes.
 *
 * @details A folder's recent-tab indexes, tab data and the container's size lists are written
 * with QDataStream and kept as byte arrays. serialize() and deserialize() are the two
 * directions of that round trip, for any type QDataStream can write and read.
 *
 * @note The struct is never instantiated; both functions are static. It is a docking-local
 * copy of the host's equivalent helper, kept separate so this library depends on Qt only.
 * @see QcwTabFolder, QcwTabFolderContainer
 */
struct QcwDockSerializeUtils
{
    /**
     * @brief Serializes an object of type T into a QByteArray.
     *
     * This template helper uses QDataStream to serialize the given object @p t
     * into a byte array. The type T must support the left-shift operator (<<)
     * for QDataStream.
     *
     * @tparam T The type of the object to serialize. Must be serializable via QDataStream.
     * @param t The constant reference to the object to be serialized.
     * @return A QByteArray containing the serialized binary data.
     *
     * @warning Ensure that the type T is properly registered or supported by QDataStream.
     *          For custom types, you must implement insertion operator (<<) for QDataStream.
     */
    template <typename T>
    static QByteArray serialize(const T& t)
    {
        QByteArray byteArray;
        QBuffer buffer(&byteArray);
        buffer.open(QIODevice::WriteOnly);
        QDataStream out(&buffer);
        out << t;
        return byteArray;
    }

    /**
     * @brief Deserializes a QByteArray back into an object of type T.
     *
     * This template helper uses QDataStream to deserialize data from a byte array
     * into an object of type T. The type T must have a default constructor and
     * support the extraction operator (>>) for QDataStream.
     *
     * @tparam T The type of the object to deserialize. Must be deserializable via QDataStream.
     * @param byteArray The byte array containing the serialized data.
     * @return An object of type T reconstructed from the byte array.
     *
     * @warning If the byte array is corrupted or does not match the expected format
     *          for type T, the behavior is undefined or may result in a partially
     *          constructed object. Ensure data integrity before calling.
     */
    template <typename T>
    static T deserialize(QByteArray byteArray)
    {
        T t;
        QBuffer buffer(&byteArray);
        buffer.open(QIODevice::ReadOnly);
        QDataStream in(&buffer);
        in >> t;
        return t;
    }
};

QT_END_NAMESPACE
