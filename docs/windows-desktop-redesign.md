# Windows desktop redesign

Target: Okular 26.08.3 on `custom/26.08-windows` in `tsiayigong/okular`.
The approved reference is the 84-screen prototype at
`output/playwright/okular-app-design`, design commit `cd87f03`.

## Implemented

- PingFang SC Regular and the FreeType Windows font engine.
- Compact light/dark palettes, menus, tabs and toolbar controls.
- Welcome and empty-history layouts, recent-document cards, keyboard activation,
  history clearing and document drop targets.
- Application menu, grouped navigation/bookmarks and shared actions that follow
  the active document. Windows chrome uses Qt's expanded client area and the DWM
  acrylic backdrop; packaged Windows rendering remains to be checked.
- Main toolbar overlay with an 8 px reveal area, delayed hiding, keyboard access,
  and stable document geometry across document and tab changes.
- Full-width document tabs above the sidebar, an open-document button, and a
  compact text selector for the sidebar panels. Thumbnails use page outlines
  and page-number badges to indicate selection.
- Soft white paper (`#FAFAF8`) shared by document rendering and thumbnails.
- Appearance settings, three built-in presets, custom preset creation, version 1
  JSON import with preview, accent color, paper material, opacity and blur.
- Searchable settings navigation, separate General and Reading pages, grouped
  controls and a shared page heading. Reading preferences retain their existing
  persisted values, and the outer canvas color is available under Appearance.
- Paper materials use the same renderer in their preview, pages and thumbnails.
  Acrylic combines a tinted scene or a selected local image with blur. Explicit
  PDF backgrounds are handled by the planned experimental background processor.
- Bookmark rename popover anchored beside the selected row; F2 and the context
  menu use the same editor.
- Selection actions for copy, continuous text, highlight, comments and speech.
  Text comments store their selected range as PDF markup; the note editor derives
  the quote from that range after reopening, alongside the editable comment.
- Floating presentation controls with a laser pointer, pen, translucent
  highlighter, stroke eraser, per-page undo/clear, colors and stroke widths.
  Existing drawing presets remain available. Ending a presentation offers to
  retain its ink as document annotations, discard it, or continue presenting.
- Windows Craft build and standalone ZIP packaging on the fork's GitHub Actions.

## Remaining implementation and validation

| Reference screens | Work remaining |
| --- | --- |
| 03–06 | Open/password/error/loading presentation |
| 07–17, 66–71, 83–84 | Sidebar panel contents, reading states, dark surfaces and Windows acrylic validation |
| 18–38 | Restyle and validate the existing annotation, form, signature, attachment, document information, save/export/print/share flows |
| 77, 80–81 | Presenter and audience windows, display assignment, Beamer note layouts, timer and next-slide preview |
| 41–55 | Complete the unified settings pages and their shortcuts/toolbars/language integrations |
| 56–65 | Complete menu grouping, help, credits and component licenses |
| 72–76 | Refine appearance library layout; implement per-document experimental PDF background recognition and comparison |
| All | Windows build, packaged runtime checks, high-DPI and multi-display validation against the reference |

## Validation

Local Qt tests cover real PDF page/thumbnail colors, theme application, toolbar
geometry, preset creation/application/cancellation, JSON schema validation,
and cropped material rendering. PDF selection tests cover comment creation,
undo/redo, save/reopen, and recovery of the selected quote.
Shell tests also cover menu order across document switches, tab geometry and
visibility, close/undo, the recent-file open/close/clear flow, and finding,
applying and cancelling reading preferences through the settings window.
Presentation tests cover laser hover/click behavior, stroke erasure, undo/clear,
page-local history, cancellation on exit, and retained ink after PDF save/reopen.
Local captures are under `output/okular-native`.
Windows build 42378210987 passed compilation, executable version checking and
standalone packaging at commit `f58846d`. Packaged GUI rendering and full
reference coverage remain pending.
