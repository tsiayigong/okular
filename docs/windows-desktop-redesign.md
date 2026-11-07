# Windows desktop redesign

Target: Okular 26.08.3 on `custom/26.08-windows` in `tsiayigong/okular`.
The approved reference is the 84-screen prototype at
`output/playwright/okular-app-design`, design commit `cd87f03`.

## Implemented

- PingFang SC Regular and the FreeType Windows font engine.
- Compact light/dark palettes, menus, tabs and toolbar controls.
- Main toolbar overlay with an 8 px reveal area, delayed hiding, keyboard access,
  and stable document geometry across document and tab changes.
- Soft white paper (`#FAFAF8`) shared by document rendering and thumbnails.
- Appearance settings, three built-in presets, custom preset creation, version 1
  JSON import with preview, accent color, paper material, opacity and blur.
- Paper materials use the same renderer in their preview, pages and thumbnails.
  Acrylic combines a tinted scene or a selected local image with blur. Explicit
  PDF backgrounds are handled by the planned experimental background processor.
- Bookmark rename popover anchored beside the selected row; F2 and the context
  menu use the same editor.
- Selection actions for copy, continuous text, highlight, comments and speech.
  Text comments store their selected range as PDF markup; the note editor derives
  the quote from that range after reopening, alongside the editable comment.
- Windows Craft build and standalone ZIP packaging on the fork's GitHub Actions.

## Remaining implementation and validation

| Reference screens | Work remaining |
| --- | --- |
| 01–06 | Welcome, recent files, open/password/error/loading presentation |
| 07–17, 66–71, 83–84 | Unified window chrome, native Windows acrylic, sidebar controls, reading states and dark surfaces |
| 18–38 | Restyle and validate the existing annotation, form, signature, attachment, document information, save/export/print/share flows |
| 39–40, 77–82 | Presentation laser/highlighter, presenter and audience windows, Beamer note layouts, ink retention |
| 41–55 | Complete the unified settings pages and their shortcuts/toolbars/language integrations |
| 56–65 | Reorganize application menus and help; review credits and component licenses |
| 72–76 | Refine appearance library layout; implement per-document experimental PDF background recognition and comparison |
| All | Windows build, packaged runtime checks, high-DPI and multi-display validation against the reference |

## Validation

Local Qt tests cover real PDF page/thumbnail colors, theme application, toolbar
geometry, preset creation/application/cancellation, JSON schema validation,
and cropped material rendering. PDF selection tests cover comment creation,
undo/redo, save/reopen, and recovery of the selected quote.
Local captures are under `output/okular-native`.
Windows runtime and full reference coverage remain pending.
