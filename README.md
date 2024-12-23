wxStartPage
=============================

<img src="img/RegularTheme.png" width="75%" />

[![Linux Build](https://github.com/Blake-Madden/wxStartPage/actions/workflows/unix%20build.yml/badge.svg)](https://github.com/Blake-Madden/wxStartPage/actions/workflows/unix%20build.yml)
[![macOS Build](https://github.com/Blake-Madden/wxStartPage/actions/workflows/macos-build.yml/badge.svg)](https://github.com/Blake-Madden/wxStartPage/actions/workflows/macos-build.yml)

[![cppcheck](https://github.com/Blake-Madden/wxStartPage/actions/workflows/cppcheck.yml/badge.svg)](https://github.com/Blake-Madden/wxStartPage/actions/workflows/cppcheck.yml)
[![doxygen](https://github.com/Blake-Madden/wxStartPage/actions/workflows/doxygen.yml/badge.svg)](https://github.com/Blake-Madden/wxStartPage/actions/workflows/doxygen.yml)
[![quneiform](https://github.com/Blake-Madden/wxStartPage/actions/workflows/i18n-check.yml/badge.svg)](https://github.com/Blake-Madden/wxStartPage/actions/workflows/i18n-check.yml)


About
=============================

A [wxWidgets](https://github.com/wxWidgets/wxWidgets) landing page for an application.

It displays a most-recently-used file list on the right side (up to 9 files) and a list of
customizable buttons on the left.

- Users clicking on these buttons can be handled via a `wxEVT_STARTPAGE_CLICKED` event.
- Modified dates are shown next to each file in the MRU list.
  These dates are shown in a human readable format (e.g., "Just now", "12 minutes ago",
  "Yesterday", "Tues at 1:07 PM").
- Human readable file paths are shown below the files (e.g., "Documents » Invoices")
- Includes a "Clear file list" button beneath the MRU list.
- The application name & logo can be shown above the custom buttons.
- A dynamic greeting (based on the time of day) is shown above the MRU list.
  This greeting is also customizable (e.g., can optionally include the user's name).
- Customizable theming.
- Supports dark mode.
- Supports HiDPI displays.

Dark theme:

<img src="img/DarkTheme.png" width="75%" />

Button hover effect:

<img src="img/HoverButton.png" width="150"/>

Button hover 3D effect:

<img src="img/HoverButton3D.png" width="150"/>

Building
=============================

Download the repository:

```
git clone https://github.com/Blake-Madden/wxStartPage.git
```

Then, include `startpage.h` and `startpage.cpp` in your project.

To build the demo, build `demo/CMakeLists.txt` with CMake and your compiler of choice.

Documentation
-----------------------------

To build the API documentation, open `Doxyfile` in Doxygen and run it.

Dependencies
=============================

- [wxWidgets](https://github.com/wxWidgets/wxWidgets) 3.2 or higher
- A C++17 compatible compiler
