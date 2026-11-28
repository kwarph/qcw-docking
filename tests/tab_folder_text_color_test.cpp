// SPDX-FileCopyrightText: 2026 Cheng Lei <kwarph@users.noreply.github.com>
// SPDX-License-Identifier: BSD-3-Clause

// Per-tab text color: the QcwTabFolder API, painting through the renderer, and the state
// transfer on move/takeTab. Dirty state only adds the '*' marker; it never changes the color.

#include "QcwTabFolder.h"
#include "QcwTabFolderRenderer.h"

#include <QApplication>
#include <QColor>
#include <QImage>
#include <QPainter>
#include <QWidget>
#include <cstdio>

namespace
{

int failures = 0;

void check(bool condition, const char* message)
{
    if (!condition)
    {
        std::fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

QImage renderFolder(QcwTabFolder& folder)
{
    QImage image { folder.size(), QImage::Format_ARGB32_Premultiplied };
    image.fill(Qt::transparent);
    QPainter painter { &image };
    folder.render(&painter);
    painter.end();
    return image;
}

int greenTextPixels(const QImage& image)
{
    int count = 0;
    for (int y = 0; y < image.height(); ++y)
    {
        for (int x = 0; x < image.width(); ++x)
        {
            const QColor color = image.pixelColor(x, y);
            if (color.green() > 180 && color.red() < 120 && color.blue() < 120)
            {
                ++count;
            }
        }
    }
    return count;
}

} // namespace

int main(int argc, char** argv)
{
    QApplication app { argc, argv };

    QcwTabFolder folder;
    folder.resize(420, 140);
    folder.addTab(new QWidget, QStringLiteral("Alpha"));
    folder.addTab(new QWidget, QStringLiteral("Beta"));
    folder.show();
    QApplication::processEvents();

    check(!folder.tabTextColor(0).isValid(), "a new tab has no explicit text color");
    check(!folder.tabTextColor(1).isValid(), "every new tab starts with the renderer default");
    check(!folder.tabTextColor(-1).isValid() && !folder.tabTextColor(99).isValid(),
          "an invalid index reports an invalid color");

    folder.setTabTextColor(-1, QColor(Qt::red));
    folder.setTabTextColor(99, QColor(Qt::red));
    check(!folder.tabTextColor(0).isValid(), "an invalid index ignores setTabTextColor");

    const QImage defaultImage = renderFolder(folder);
    check(greenTextPixels(defaultImage) == 0, "the default tab text is not green");

    const QColor green { 0x00, 0xFF, 0x00 };
    folder.setTabTextColor(0, green);
    check(folder.tabTextColor(0) == green, "setTabTextColor stores the explicit color");
    check(!folder.tabTextColor(1).isValid(), "setting one tab leaves the other tab alone");

    const QImage coloredImage = renderFolder(folder);
    check(greenTextPixels(coloredImage) > 0, "the renderer paints the tab with its explicit color");

    folder.setTabTextColor(0, QColor());
    check(!folder.tabTextColor(0).isValid(), "an invalid color restores the renderer default");
    check(greenTextPixels(renderFolder(folder)) == 0, "clearing the color restores the default text");

    // Reordering keeps the color with its tab.
    folder.setTabTextColor(0, green);
    folder.moveTab(0, 1);
    check(!folder.tabTextColor(0).isValid() && folder.tabTextColor(1) == green, "a moved tab carries its text color");

    // Taking a tab into another folder clones the color with the rest of the tab state.
    QcwTabFolder other;
    int adopted = -1;
    folder.takeTab(&other, 1, adopted);
    check(adopted >= 0 && other.tabTextColor(adopted) == green,
          "takeTab() carries the text color to the adopting folder");

    folder.setTabDirty(0, true);
    check(!folder.tabTextColor(0).isValid(), "the dirty marker does not invent a text color");

    return failures == 0 ? 0 : 1;
}
