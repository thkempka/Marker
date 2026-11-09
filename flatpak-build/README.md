# Local Flatpak test build (TOC branch)

Flatpak manifest that builds the `feature/toc-sidebar` branch of this fork
(plus the auto-reload fix) against gtksourceview-4 + webkit2gtk-4.1, for
local testing on GNOME 50 runtime.

Build (needs flathub's org.gnome.Platform/Sdk//50):

    flatpak-builder --ccache --repo=repo --force-clean build-dir \
        com.github.fabiocolacio.marker.json
    flatpak install --user ./repo com.github.fabiocolacio.marker

Patches/: auto-reload fix (see branch fix/preview-auto-reload-on-open).
