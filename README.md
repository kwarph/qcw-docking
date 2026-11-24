# qcw-docking

Qt docking system: tabbed folders, a splitter container that arranges them,
floating folders, minimized fast views and a pluggable folder renderer. The library depends on
Qt Widgets only — it does not know about a workbench, perspectives or views, so the host
application owns everything above the folder level.

## Components

| Component | Role |
|---|---|
| `QcwTabFolder` | A tab bar plus one pane: tab close/detach/dirty state, tab-list popup with a filter, MRU switching, maximize/minimize, a folder settings dialog, a right-aligned widget in the tab bar |
| `QcwTabFolderContainer` | The splitter tree: `createFolder()`, central folder, save/restore of the layout as XML |
| `QcwTabFolderSite` | The splitter-side stub that keeps ratios while a folder is replaced |
| `QcwTabPaneManager` | Process-wide id assignment and reference counting for panes shared between folders |
| `QcwTabFolderRenderer` | Base renderer; subclasses paint the chrome, the default implementation is the fallback |
| `QcwTabFolderRendererCreators` | Process-wide name → renderer factory registry |
| `QcwFloatingFolderMonitor` | Tracks folders that float into their own top-level windows |
| `QcwFastViewToolBar` | The window-edge tool bar a minimized folder turns into |
| `QcwFastViewStyleSettings` | Process-wide margin/border values the host writes before a fast view opens |
| `QcwTabListWidget` | The list widget used by the tab-list popup: filter line, dirty marker/color and an item context-menu signal shared with the folder's tab menu |

## Requirements

- C++17 compiler, CMake 3.23+
- Qt 6.9+ with the Widgets module

## Build

```sh
cmake -S . -B build -DQCWDOCK_BUILD_TESTS=ON -DQCWDOCK_BUILD_EXAMPLES=ON
cmake --build build -j
ctest --test-dir build --output-on-failure
```

The examples land in `build/bin`:

- `folder_container_demo` — three folders in a container, layout save/restore, floating
  folders, a container inside a dialog.
- `folder_renderer_demo` — two custom renderers and a preferences dialog that switches between
  them.

When the project is added as a subdirectory of a host, `QCWDOCK_BUILD_TESTS` and
`QCWDOCK_BUILD_EXAMPLES` default to `OFF`; only the library is built.

## Using the library

As a subdirectory:

```cmake
add_subdirectory(third_party/qcw-docking)
target_link_libraries(myapp PRIVATE QCWDOCK::qcwdocking)
```

Or as an installed package:

```cmake
find_package(qcwdocking 1.0 REQUIRED)
target_link_libraries(myapp PRIVATE QCWDOCK::qcwdocking)
```

Include the installed headers as `#include <QcwTabFolder.h>`; they live in
`include/qcwdocking`.

The demo applications show the intended layering: the host creates the panes, hands them to
`QcwTabFolder::addTab()`, and rebuilds them itself when `QcwTabFolderContainer::restoreState()`
asks for panes by id. See [`examples/folder_container_demo/README.md`](examples/folder_container_demo/README.md).

## Icons and translations

The default renderer asks `QcwTabFolderRenderer::setIconProvider()` for its maximize, restore,
minimize and view icons. Without a provider it falls back to the current style's standard
icons. A host that wants its own icon set installs a provider once, before folders are created.

The library ships `translations/qcwdocking_<locale>.ts`/`.qm` and does not install a
`QTranslator` itself. A host registers the `qcwdocking` catalog base with its own translation
loader; a standalone application that does not is simply English.

## Versioning

`CMakeLists.txt` holds the single version number, and a release tag is that number prefixed
with `v` (`v1.1.1`). Semantic versioning applies to the public API — the `QCWDOCK_EXPORT`
classes and functions, the installed headers and the renderer interface. A host that links
this library through `find_package()` should require the matching major version.

QCW 2.x requires qcwdocking 1.x.

## License

BSD-3-Clause. See [`LICENSE`](LICENSE).
