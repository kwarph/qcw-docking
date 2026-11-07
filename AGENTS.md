# qcw-docking — project conventions for agents

## Scope

This repository is the standalone docking library. It depends on Qt 6 Widgets and nothing
else. Do not add includes of host-workbench classes (`QcwWindow`, `QcwPerspective`, `QcwView`,
`QcwWorkbench`, `QcwWindowStyler`, `QcwSessionData`): the host depends on the folder, never the
other way round. Coupling to a host runs through dynamic properties (`ViewId`, `WindowId`,
`FolderId`), the pane ids, and the renderer/icon hooks described below.

## Export macro

Every exported class includes `QcwDockingGlobal.h` and uses `QCWDOCK_EXPORT`; the library
target defines `QCWDOCK_LIBRARY` while building. A static build relies on
`QCWDOCK_STATIC_LIBRARY`. Debug-only diagnostics use `QCWDOCK_DEBUG`, not `QCW_DEBUG`.

## Singletons

Four process-wide objects follow the repository pattern:

| Class | Accessor macro |
|---|---|
| `QcwFastViewStyleSettings` | `fastViewStyleSettings` |
| `QcwFloatingFolderMonitor` | `floatingFolderMonitor` |
| `QcwTabFolderRendererCreators` | `folderRendererCreators` |
| `QcwTabPaneManager` | `tabPaneManager` |

```cpp
class QCWDOCK_EXPORT QcwFastViewStyleSettings
{
public:
    static QcwFastViewStyleSettings* instance();
    // ...
};

#ifndef fastViewStyleSettings
#define fastViewStyleSettings QcwFastViewStyleSettings::instance()
#endif

// source
Q_GLOBAL_STATIC(QcwFastViewStyleSettings, fastViewStyleSettingsInstance);
```

- `instance()` always returns a valid object; call sites do not null-check it.
- Keep the lowerCamelCase macro; it is the intended way to reach the global.
- `QcwFastViewStyleSettings` and `QcwTabFolderRendererCreators` are plain classes, not
  `QObject`s. `QcwFloatingFolderMonitor` and `QcwTabPaneManager` are `QObject`s with no parent,
  because they outlive everything that connects to them.
- A second instance is pointless: nothing connects to it and every lookup goes through the
  macro.

Ownership:

- `QcwFastViewStyleSettings` and `QcwTabFolderRendererCreators` store copies — two ints/bools,
  and name-to-`std::function` entries. Registering a name again replaces the creator.
- `QcwFloatingFolderMonitor` keeps no state.
- `QcwTabPaneManager` deletes the panes it tracks: a pane is reference-counted across the
  folders showing it, and `removeReference()` calls `deleteLater()` when the last folder lets
  go. `clear(true)` deletes every tracked pane at session teardown. Panes that die without a
  release are dropped through their `destroyed()` signal.
- A renderer created through `QcwTabFolderRendererCreators` returns with ownership at the
  caller; `QcwTabFolder::setRenderer()` takes it over.

## Renderer and icons

`QcwTabFolderRenderer` is the public base. `QcwDefaultTabFolderRenderer` is the built-in
fallback, and `QcwTabFolderRendererCreators` maps the `folderRenderer` name of a host style to
a factory. Renderer names are a host contract; do not rename the built-in one casually.

The base icon methods call the process-wide provider installed with
`QcwTabFolderRenderer::setIconProvider()` and fall back to the current style's standard icons
(`SP_TitleBarNormalButton`, `SP_TitleBarMaxButton`, `SP_TitleBarMinButton`,
`SP_FileDialogListView`) when no provider is set. The provider is GUI-thread state, installed
once before folders are created. `renderMaximizeIcon()`, `renderRestoreIcon()`,
`renderMinimizeIcon()` and `renderViewIcon()` stay virtual so subclasses keep working.

## Serialization

`QcwDockSerializeUtils` is the serialization helper for the folder/container save documents
(`serialize()`/`deserialize()` over `QDataStream`). The struct is deliberately named
differently from a host's own helper: two identically-named structs across two shared libraries
would be an ODR violation. A saved `<tabPane>` stores the arguments that created the pane, its
type and its id; the host rebuilds panes from that, while this library only restores tabs and
layout.

`QcwDockingTypes.h` holds the enums, flags and the `TabPaneIdPropertyName` contract that the
serialized documents depend on. Do not change an existing enum value or property name without
treating it as a format break.

## Translations

`translations/qcwdocking_<locale>.ts` and the compiled `.qm` files ship with the library and
install to `share/qcw/translations`. This library does not create a `QTranslator` and has no
translation registry: the host that owns the application registers the `qcwdocking` catalog
base with its own loader. Keep new user-visible strings in `tr()`/`QCoreApplication::translate`
with the class name as context, then regenerate the catalogs with `lupdate` and `lrelease`.

## Tests

The suite is plain `main()` programs that print `FAIL:` lines and return non-zero; they are not
QtTest classes. Every test runs with `QT_QPA_PLATFORM=offscreen` and
`QCWDOCK_TEST_AUTOCLOSE=1`, except `docking_drag_drop_test`, which needs the `minimal` platform
because the offscreen plugin's drag implementation does not run the `QDrag::exec()` loop. Keep
the environment properties in `tests/CMakeLists.txt` when adding a test that enters
`QApplication::exec()`.

## Build and versioning

CMake 3.23+, C++23, Qt 6 Widgets only. `QCWDOCK_BUILD_TESTS`/`QCWDOCK_BUILD_EXAMPLES` default
to ON only when the project is top-level, so a host's `add_subdirectory()` builds the library
alone. `CMakeLists.txt` holds the single version number; a release tag is `v` plus that number.
Semantic versioning applies to the `QCWDOCK_EXPORT` API, the installed headers and the renderer
interface. Public API additions bump the minor digit; removals, renamed or changed signatures
and ownership changes bump the major one.
