// A pane can be shown by several folders, but only the folder that parents it may move it.
// A style change reaches hidden fast-view folders too; without the ownership guard the
// hidden folder resized the shared pane to its own tiny page and the visible view went blank.

#include "QcwTabFolder.h"

#include <QApplication>
#include <QEvent>
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

} // namespace

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QcwTabFolder host;
    host.resize(600, 400);
    QWidget pane;
    host.addTab(&pane, "Host");
    host.show();
    app.processEvents();
    host.setCurrentIndex(0);
    app.processEvents();

    check(pane.parentWidget() == &host, "the visible folder parents its pane");
    const QRect hostedGeometry = pane.geometry();

    // Park the same pane in a small, hidden folder. Its current pointer names the pane even
    // though the visible folder is the one that parents it.
    QcwTabFolder parked;
    parked.resize(120, 30);
    parked.addTab(&pane, "Parked");
    parked.setCurrentIndex(0);
    app.processEvents();

    host.setCurrentIndex(-1);
    host.setCurrentIndex(0);
    app.processEvents();
    check(pane.parentWidget() == &host, "the visible folder reclaims the shared pane");

    const QRect beforeStyleChange = pane.geometry();
    QEvent styleChange { QEvent::StyleChange };
    QCoreApplication::sendEvent(&parked, &styleChange);
    app.processEvents();

    check(pane.parentWidget() == &host, "a hidden folder does not take the pane back on a style change");
    check(pane.geometry() == beforeStyleChange,
          "a hidden folder does not resize a shared pane it does not parent");

    // Showing the parked folder takes the pane, and the host takes it back when it becomes
    // current again. A stale current pointer must not short-circuit the reclaim.
    parked.show();
    parked.setCurrentIndex(0);
    app.processEvents();
    check(pane.parentWidget() == &parked, "the newly visible folder takes the shared pane");

    host.setCurrentIndex(0);
    app.processEvents();
    check(pane.parentWidget() == &host, "the host takes the shared pane back");
    check(pane.geometry() == hostedGeometry, "the reclaimed pane gets the host's geometry");

    if (failures == 0)
    {
        std::fprintf(stderr, "passed\n");
    }
    return failures == 0 ? 0 : 1;
}
