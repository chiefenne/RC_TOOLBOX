# Adding a Screen

!!! note "Under Development"
    This developer documentation is being written.

## Overview

Pages inherit from `PageBase` (see `gui/page_base.h`).

## Steps

1. Create `gui/pages/page_xxx.cpp` and `page_xxx.h`
2. Add `PAGE_XXX` to `GuiPage` enum in `gui/gui.h`
3. Add case in `gui_set_page()` in `gui/gui.cpp`
4. Add string IDs for page title in `gui/lang.h` and translations
5. Add navigation button in `page_home.cpp` if needed

*Detailed examples coming soon.*
