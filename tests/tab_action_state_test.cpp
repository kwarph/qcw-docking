// Drives QcwTabFolder::setTabActionState(): how Enabled/Disabled/Hidden override the
// per-click visibility rules of the tab bar context menu.
//
// The menu is modal, so every probe arms a zero timer that fires inside QMenu::exec(),
// records that the menu was up and closes it. The QAction flags are read after the close,
// because QMenu does not touch them: they are the flags the menu was opened with.

#include "QcwDockingTypes.h"
#include "QcwTabFolder.h"

#include <QAction>
#include <QApplication>
#include <QContextMenuEvent>
#include <QMenu>
#include <QTimer>
#include <QWidget>
#include <cstdio>
#include <memory>

namespace
{

int failures = 0;

/// Every standard entry of the tab bar context menu, in the order they are declared.
const QLatin1String standardIds[] = { QcwTabActionIds::Close,         QcwTabActionIds::CloseOthers,
                                      QcwTabActionIds::CloseLeft,     QcwTabActionIds::CloseRight,
                                      QcwTabActionIds::CloseAll,      QcwTabActionIds::Detach,
                                      QcwTabActionIds::FolderSettings };

void check(bool condition, const char* message)
{
    if (!condition)
    {
        std::fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

/// The centre of the tab at @p index, found by scanning the bar because tab geometry is private.
QPoint tabBarPoint(QcwTabFolder& folder, int index)
{
    QRect rect;
    for (int y = 0; y < folder.height(); ++y)
    {
        for (int x = 0; x < folder.width(); ++x)
        {
            if (folder.indexAt(QPoint(x, y)) == index)
            {
                rect |= QRect(x, y, 1, 1);
            }
        }
    }

    return rect.isNull() ? QPoint(-1, -1) : rect.center();
}

struct MenuProbe
{
    bool opened { false };
};

/// Right-clicks the tab at @p index, lets the folder open its context menu and closes it.
MenuProbe openTabMenu(QcwTabFolder& folder, int index)
{
    MenuProbe probe;
    const QPoint pos = tabBarPoint(folder, index);
    check(pos != QPoint(-1, -1), "the tab under the cursor is reachable in the bar");

    QAction* close = folder.tabAction(QcwTabActionIds::Close);
    QTimer::singleShot(0, [&probe, close]()
    {
        for (QWidget* widget : QApplication::topLevelWidgets())
        {
            auto* menu = qobject_cast<QMenu*>(widget);
            // The folder's menu is the one holding its standard entries; other popups are
            // not this test's business.
            if (!menu || !menu->isVisible() || !menu->actions().contains(close))
            {
                continue;
            }

            probe.opened = true;
            menu->close();
            return;
        }
    });

    QContextMenuEvent event(QContextMenuEvent::Mouse, pos, folder.mapToGlobal(pos));
    QApplication::sendEvent(&folder, &event);
    // A probe whose menu never opened leaves its timer pending; fire it here, while no menu
    // is up, so it cannot close the next probe's menu.
    QApplication::processEvents();
    return probe;
}

QcwTabFolder* makeFolder(int tabCount = 3)
{
    auto* folder = new QcwTabFolder;
    folder->resize(700, 300);

    for (int i = 0; i < tabCount; ++i)
    {
        folder->addTab(new QWidget, QStringLiteral("Tab %1").arg(i + 1));
    }

    folder->setCurrentIndex(0);
    folder->show();
    QApplication::processEvents();
    return folder;
}

void testDefaults()
{
    std::fprintf(stderr, "--- defaults ---\n");

    QcwTabFolder folder;
    for (const auto& id : standardIds)
    {
        check(folder.tabAction(id) != nullptr, "every standard id names an action");
        check(folder.tabActionState(id) == qcw::TabActionState::Enabled, "a fresh folder starts Enabled");
    }

    const QLatin1String unknown { "QcwTabFolder/notASstandardAction" };
    check(folder.tabAction(unknown) == nullptr, "an unknown id names no action");
    check(!folder.setTabActionState(unknown, qcw::TabActionState::Hidden), "an unknown id is refused");
    check(folder.tabActionState(unknown) == qcw::TabActionState::Enabled, "an unknown id reports Enabled");
}

void testHiddenSurvivesTheMenu()
{
    std::fprintf(stderr, "--- hidden entry ---\n");

    std::unique_ptr<QcwTabFolder> folder(makeFolder());

    QAction* detach = folder->tabAction(QcwTabActionIds::Detach);
    check(folder->setTabActionState(QcwTabActionIds::Detach, qcw::TabActionState::Hidden), "Hide is accepted");
    check(folder->tabActionState(QcwTabActionIds::Detach) == qcw::TabActionState::Hidden, "the state sticks");
    check(!detach->isVisible(), "hiding takes effect at once");

    const MenuProbe probe = openTabMenu(*folder, 0);
    check(probe.opened, "the menu still opens for its remaining entries");
    check(!detach->isVisible(), "the per-click update does not bring a hidden entry back");
    check(folder->tabAction(QcwTabActionIds::Close)->isVisible(), "the neighbouring entries stay");
    check(folder->tabAction(QcwTabActionIds::CloseOthers)->isVisible(), "the sibling close entries stay");

    // The other tab has the same detach rule, so a hidden entry stays hidden there too.
    const MenuProbe second = openTabMenu(*folder, 1);
    check(second.opened, "the menu opens on another tab");
    check(!detach->isVisible(), "the entry stays hidden on another tab");
}

void testDisabledKeepsApplicability()
{
    std::fprintf(stderr, "--- disabled entry ---\n");

    std::unique_ptr<QcwTabFolder> folder(makeFolder());
    folder->setTabClosable(1, false);

    QAction* close = folder->tabAction(QcwTabActionIds::Close);
    check(folder->setTabActionState(QcwTabActionIds::Close, qcw::TabActionState::Disabled), "Disable is accepted");
    check(folder->tabActionState(QcwTabActionIds::Close) == qcw::TabActionState::Disabled, "the state sticks");
    check(!close->isEnabled(), "disabling takes effect at once");

    const MenuProbe closable = openTabMenu(*folder, 0);
    check(closable.opened, "a disabled entry keeps the menu open");
    check(close->isVisible(), "a disabled entry that applies is still listed");
    check(!close->isEnabled(), "the per-click update does not re-enable it");

    const MenuProbe locked = openTabMenu(*folder, 1);
    check(locked.opened, "the menu opens on the non-closable tab");
    check(!close->isVisible(), "a disabled entry that does not apply stays hidden");
    check(!close->isEnabled(), "it is still disabled as well");
}

void testEnabledGivesTheEntryBack()
{
    std::fprintf(stderr, "--- back to enabled ---\n");

    std::unique_ptr<QcwTabFolder> folder(makeFolder());
    QAction* detach = folder->tabAction(QcwTabActionIds::Detach);
    folder->setTabActionState(QcwTabActionIds::Detach, qcw::TabActionState::Disabled);
    check(!detach->isEnabled(), "the initial disable worked");

    check(folder->setTabActionState(QcwTabActionIds::Detach, qcw::TabActionState::Enabled), "Enable is accepted");
    check(detach->isEnabled(), "Enabled gives the enabled flag back");

    const MenuProbe probe = openTabMenu(*folder, 0);
    check(probe.opened, "the menu opens again");
    check(detach->isVisible(), "Enabled returns the entry to the per-click rules");
}

void testBareActionFlags()
{
    std::fprintf(stderr, "--- bare QAction flags ---\n");

    std::unique_ptr<QcwTabFolder> folder(makeFolder());
    QAction* detach = folder->tabAction(QcwTabActionIds::Detach);

    // Without a state the old contract holds: a bare disable survives the per-click update,
    // a bare hide does not.
    detach->setEnabled(false);
    detach->setVisible(false);

    const MenuProbe probe = openTabMenu(*folder, 0);
    check(probe.opened, "the menu opens");
    check(detach->isVisible(), "a bare hide is still only a temporary hide");
    check(!detach->isEnabled(), "a bare disable still survives the per-click update");
}

void testNoMenuWhenEverythingIsHidden()
{
    std::fprintf(stderr, "--- nothing left to offer ---\n");

    std::unique_ptr<QcwTabFolder> folder(makeFolder());

    for (const auto& id : standardIds)
    {
        folder->setTabActionState(id, qcw::TabActionState::Hidden);
    }

    const MenuProbe none = openTabMenu(*folder, 0);
    check(!none.opened, "hiding every standard entry opens no menu at all");

    // An application entry added through tabMenu() is what keeps the menu alive then.
    folder->tabMenu()->addAction(QStringLiteral("Custom action"));
    const MenuProbe custom = openTabMenu(*folder, 0);
    check(custom.opened, "a visible custom action still opens the menu");
}

} // namespace

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    testDefaults();
    testHiddenSurvivesTheMenu();
    testDisabledKeepsApplicability();
    testEnabledGivesTheEntryBack();
    testBareActionFlags();
    testNoMenuWhenEverythingIsHidden();

    if (failures == 0)
    {
        std::printf("tab_action_state_test: all checks passed\n");
    }
    else
    {
        std::fprintf(stderr, "tab_action_state_test: %d failure(s)\n", failures);
    }

    return failures == 0 ? 0 : 1;
}
