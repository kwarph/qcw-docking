// QcwTabListWidget: dirty marker/color, item context-menu signal and the tab list popup
// staying inside the available screen area.

#include "QcwTabFolder.h"
#include "QcwTabFolderRenderer.h"
#include "QcwTabListWidget.h"

#include <QApplication>
#include <QAction>
#include <QContextMenuEvent>
#include <QFont>
#include <QLineEdit>
#include <QListView>
#include <QMenu>
#include <QScreen>
#include <QStandardItem>
#include <QTimer>
#include <QToolButton>
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

void testDirtyItem()
{
    std::fprintf(stderr, "--- dirty item ---\n");

    QcwTabListWidget list(true);
    const QFont font;
    QStandardItem* clean = list.addItem(QStringLiteral("clean.cpp"), QIcon(), font);
    QStandardItem* dirty =
        list.addItem(QStringLiteral("dirty.cpp"), QIcon(), font, true, QColor(QStringLiteral("#3366cc")));

    check(clean->text() == QStringLiteral("clean.cpp"), "a clean item keeps its text");
    check(!clean->data(Qt::ForegroundRole).isValid(), "a clean item keeps the palette foreground");
    check(dirty->text() == QStringLiteral("*dirty.cpp"), "a dirty item is prefixed with '*'");
    check(dirty->data(Qt::ForegroundRole).value<QColor>() == QColor(QStringLiteral("#3366cc")),
          "a dirty item carries its text color");

    auto* filter = list.findChild<QLineEdit*>(QStringLiteral("filterEdit"));
    auto* view = list.findChild<QListView*>(QStringLiteral("view"));
    check(filter && view, "the list exposes its filter and view");
    if (filter && view)
    {
        filter->setText(QStringLiteral("dirty"));
        QApplication::processEvents();
        check(view->model()->rowCount() == 1, "the filter matches the name without relying on the '*' marker");
        check(view->model()->index(0, 0).data(Qt::DisplayRole).toString() == QStringLiteral("*dirty.cpp"),
              "the filtered row still shows the dirty marker");
    }
}

void testContextMenuSignal()
{
    std::fprintf(stderr, "--- item context menu ---\n");

    QcwTabListWidget list(true);
    const QFont font;
    QStandardItem* item = list.addItem(QStringLiteral("menu.cpp"), QIcon(), font);
    list.resize(320, 240);
    list.show();
    QApplication::processEvents();

    QStandardItem* reported = nullptr;
    QPoint reportedPosition;
    QObject::connect(&list, &QcwTabListWidget::itemContextMenuRequested,
                     [&reported, &reportedPosition](QStandardItem* contextItem, const QPoint& position)
    {
        reported = contextItem;
        reportedPosition = position;
    });

    auto* view = list.findChild<QListView*>(QStringLiteral("view"));
    check(view != nullptr, "the list exposes its view");
    if (!view)
    {
        return;
    }

    const QRect rect = view->visualRect(view->model()->index(0, 0));
    const QPoint local = rect.center();
    QContextMenuEvent event(QContextMenuEvent::Mouse, local, view->viewport()->mapToGlobal(local));
    QApplication::sendEvent(view->viewport(), &event);

    check(reported == item, "right-clicking a row reports its item");
    check(reportedPosition == view->viewport()->mapToGlobal(local),
          "the reported position is global and points at the click");
}

void testPopupReusesTabMenu()
{
    std::fprintf(stderr, "--- popup context menu reuses tabMenu ---\n");

    QcwTabFolder folder;
    folder.resize(300, 200);
    folder.setFeature(qcw::MultiTabs, false);
    folder.addTab(new QWidget, QStringLiteral("First long tab title"));
    folder.addTab(new QWidget, QStringLiteral("Second long tab title"));
    folder.show();
    QApplication::processEvents();

    bool customTriggered = false;
    QAction* custom = folder.tabMenu()->addAction(QStringLiteral("Custom action"));
    QObject::connect(custom, &QAction::triggered, [&customTriggered]()
    {
        customTriggered = true;
    });

    auto* button = folder.findChild<QToolButton*>(QStringLiteral("tabListButton"));
    check(button && button->isVisible(), "the popup button is available");
    if (!button)
    {
        return;
    }

    button->click();
    QApplication::processEvents();
    auto* popup = folder.findChild<QcwTabListWidget*>();
    auto* view = popup ? popup->findChild<QListView*>(QStringLiteral("view")) : nullptr;
    check(popup && view, "the popup exposes its view");
    if (!popup || !view)
    {
        return;
    }

    const QRect rect = view->visualRect(view->model()->index(0, 0));
    const QPoint local = rect.center();

    QTimer::singleShot(0, [custom]()
    {
        for (QWidget* widget : QApplication::topLevelWidgets())
        {
            auto* menu = qobject_cast<QMenu*>(widget);
            if (!menu || !menu->isVisible() || !menu->actions().contains(custom))
            {
                continue;
            }
            custom->trigger();
            menu->close();
            return;
        }
    });

    QContextMenuEvent event(QContextMenuEvent::Mouse, local, view->viewport()->mapToGlobal(local));
    QApplication::sendEvent(view->viewport(), &event);
    QApplication::processEvents();

    check(customTriggered, "a custom tabMenu action is reachable from the tab list popup");
}

void testPopupStaysInsideScreen()
{
    std::fprintf(stderr, "--- popup inside screen ---\n");

    QScreen* screen = QGuiApplication::primaryScreen();
    check(screen != nullptr, "the application has a screen");
    if (!screen)
    {
        return;
    }
    const QRect available = screen->availableGeometry();

    QcwTabFolder folder;
    folder.resize(300, 200);
    folder.setFeature(qcw::MultiTabs, false);
    folder.addTab(new QWidget, QStringLiteral("A very long tab title for the list"));
    folder.addTab(new QWidget, QStringLiteral("Another very long tab title for the list"));
    folder.addTab(new QWidget, QStringLiteral("Third very long tab title for the list"));
    folder.setTabDirty(1, true);
    const QColor explicitTabColor { 0x00, 0x80, 0x00 };
    folder.setTabTextColor(1, explicitTabColor);
    check(folder.tabTextColor(1) == explicitTabColor, "setTabTextColor stores the explicit color");
    check(!folder.tabTextColor(0).isValid(), "a tab without an explicit color reports an invalid color");
    folder.move(available.right() - folder.width() + 1, available.top() + 40);
    folder.show();
    QApplication::processEvents();

    auto* button = folder.findChild<QToolButton*>(QStringLiteral("tabListButton"));
    check(button && button->isVisible(), "the hidden tabs expose the tab list button");
    if (!button)
    {
        return;
    }

    button->click();
    QApplication::processEvents();

    auto* popup = folder.findChild<QcwTabListWidget*>();
    check(popup != nullptr, "clicking the button opens the tab list popup");
    if (!popup)
    {
        return;
    }

    const QRect geometry = popup->frameGeometry();
    std::fprintf(stderr, "popup=(%d,%d %dx%d) screen=(%d,%d %dx%d)\n", geometry.x(), geometry.y(), geometry.width(),
                 geometry.height(), available.x(), available.y(), available.width(), available.height());
    check(geometry.left() >= available.left() && geometry.top() >= available.top(),
          "the popup starts inside the screen");
    check(geometry.right() <= available.right() && geometry.bottom() <= available.bottom(),
          "the popup does not cross the screen edge");

    auto* view = popup->findChild<QListView*>(QStringLiteral("view"));
    check(view != nullptr, "the popup exposes its list view");
    if (view)
    {
        const QColor expectedDirtyColor = folder.tabTextColor(1);
        bool foundDirty = false;
        bool dirtyHasColor = false;
        for (int row = 0; row < view->model()->rowCount(); ++row)
        {
            const QModelIndex index = view->model()->index(row, 0);
            const QString text = index.data(Qt::DisplayRole).toString();
            if (text.startsWith(QLatin1Char('*')))
            {
                foundDirty = true;
                dirtyHasColor = index.data(Qt::ForegroundRole).value<QColor>() == expectedDirtyColor;
            }
        }
        check(foundDirty, "the popup item shows the dirty marker");
        check(dirtyHasColor, "the popup item uses the tab's explicit text color");
    }

    popup->close();
    QApplication::processEvents();
}

} // namespace

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    testDirtyItem();
    testContextMenuSignal();
    testPopupReusesTabMenu();
    testPopupStaysInsideScreen();

    if (failures == 0)
    {
        std::printf("tab_list_widget_test: all checks passed\n");
    }
    else
    {
        std::fprintf(stderr, "tab_list_widget_test: %d failure(s)\n", failures);
    }

    return failures == 0 ? 0 : 1;
}
