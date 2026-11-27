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
    check(pane.geometry() == beforeStyleChange, "a hidden folder does not resize a shared pane it does not parent");

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

    // A visible shared pane can be parented to the parked folder while the host still names it
    // as current. Reparenting before the early-return decision would leave the host's old
    // geometry behind (the Build console's stale width).
    QcwTabFolder visibleHost;
    visibleHost.resize(600, 400);
    QWidget visiblePane;
    visibleHost.addTab(&visiblePane, "VisibleHost");
    visibleHost.show();
    app.processEvents();

    QcwTabFolder visibleParked;
    visibleParked.resize(120, 30);
    visibleParked.addTab(&visiblePane, "VisibleParked");
    visibleParked.show();
    app.processEvents();
    check(visiblePane.parentWidget() == &visibleParked, "the parked folder hosts the visible pane");

    visibleHost.setCurrentIndex(0);
    app.processEvents();
    check(visiblePane.parentWidget() == &visibleHost, "the host reclaims a visible shared pane");
    check(visiblePane.width() > visibleParked.width(), "reclaiming updates the pane to the host's width");

    // A pane already current but still carrying its construction size must still be laid out.
    visiblePane.resize(640, 450);
    visibleHost.setCurrentIndex(0);
    app.processEvents();
    check(visiblePane.width() != 640 || visibleHost.width() == 640,
          "an already-current pane is relaid out to the folder's page");

    if (failures == 0)
    {
        std::fprintf(stderr, "passed\n");
    }
    return failures == 0 ? 0 : 1;
}
