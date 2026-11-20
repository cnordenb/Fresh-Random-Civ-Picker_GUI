# Accessibility

This project strives to be compliant with [WCAG 2.2](https://www.w3.org/TR/WCAG22/) at level AA.

The checklist below has every success criterion of levels A and AA, 55 in all, in the order of WCAG 2.2; 4.1.1
Parsing is left out, as WCAG 2.2 made it obsolete. Each name links to W3C's explanation of the criterion and of how to
check it. For a user interface that is not a web page, read "page" as "screen", as
[WCAG2ICT](https://www.w3.org/TR/wcag2ict-22/) does.

Each item ends with how it is checked:

- **Automated.** A test can check it in full.
- **Partly automated; human verification required.** A test, with a tool such as axe-core for a web page, finds some
  failures, and a person checks the rest.
- **Human verification required.** A person checks it by using the interface: with the keyboard alone, a screen
  reader, zoom, or whatever else the item names.

One item can be checked by a test alone; the other 54 need a person. What a test can check belongs in the
accessibility tests of [TESTS.md](../documentation/TESTS.md).

Mark an item `[x]` when it holds for the whole user interface, and clear it again when a change touches what it is
about. An item that cannot apply, such as captions where there is no video, is marked `[x]` and ends with "Does not
apply:" and why.

## Perceivable

- [ ] [1.1.1 Non-text Content][1.1.1] (A): every image, icon, chart and control has a text alternative that serves
      the same purpose, and decoration is hidden from assistive technology.
      **Partly automated; human verification required.**
- [ ] [1.2.1 Audio-only and Video-only (Prerecorded)][1.2.1] (A): recorded audio has a transcript, and recorded video
      without sound has a transcript or an audio track that describes it.
      **Human verification required.**
- [ ] [1.2.2 Captions (Prerecorded)][1.2.2] (A): recorded video with sound has accurate captions, in step with it.
      **Human verification required.**
- [ ] [1.2.3 Audio Description or Media Alternative (Prerecorded)][1.2.3] (A): recorded video has an audio
      description, or a text alternative of everything that is seen and heard.
      **Human verification required.**
- [ ] [1.2.4 Captions (Live)][1.2.4] (AA): live video with sound has captions.
      **Human verification required.**
- [ ] [1.2.5 Audio Description (Prerecorded)][1.2.5] (AA): recorded video has an audio description of what matters on
      screen and is not said in the sound.
      **Human verification required.**
- [ ] [1.3.1 Info and Relationships][1.3.1] (A): structure shown on screen, such as headings, lists, tables, labels
      and groups, is also in the markup or otherwise exposed to assistive technology.
      **Partly automated; human verification required.**
- [ ] [1.3.2 Meaningful Sequence][1.3.2] (A): where order matters to the meaning, assistive technology reads the
      content in the right order.
      **Human verification required.**
- [ ] [1.3.3 Sensory Characteristics][1.3.3] (A): instructions do not rely only on shape, colour, size, position,
      orientation or sound, as in "press the round button on the right".
      **Human verification required.**
- [ ] [1.3.4 Orientation][1.3.4] (AA): content is not locked to portrait or landscape, unless one orientation is
      essential.
      **Partly automated; human verification required.**
- [ ] [1.3.5 Identify Input Purpose][1.3.5] (AA): fields that ask for the user's own details, such as name, e-mail or
      address, state their purpose in code, as with `autocomplete`.
      **Partly automated; human verification required.**
- [ ] [1.4.1 Use of Color][1.4.1] (A): colour is never the only way to convey information, show an action, ask for a
      response or tell elements apart.
      **Human verification required.**
- [ ] [1.4.2 Audio Control][1.4.2] (A): audio that plays by itself for more than 3 seconds can be paused or stopped,
      or its volume set apart from the system volume.
      **Partly automated; human verification required.**
- [ ] [1.4.3 Contrast (Minimum)][1.4.3] (AA): text has a contrast ratio of at least 4.5:1 with its background, and
      large text, from 18 point or 14 point bold, at least 3:1; logos, decoration and inactive controls are exempt.
      **Partly automated; human verification required.**
- [ ] [1.4.4 Resize Text][1.4.4] (AA): text can be enlarged to 200% without a screen magnifier, and no content or
      function is lost.
      **Partly automated; human verification required.**
- [ ] [1.4.5 Images of Text][1.4.5] (AA): text is shown as text, not as an image of it, except in logos or where the
      look of the text is essential.
      **Human verification required.**
- [ ] [1.4.10 Reflow][1.4.10] (AA): at a width of 320 CSS pixels, the same as 400% zoom at 1280, content needs no
      scrolling in two directions and loses nothing; maps, data tables and the like are exempt.
      **Partly automated; human verification required.**
- [ ] [1.4.11 Non-text Contrast][1.4.11] (AA): the edges and states of controls, focus indicators, and the parts of
      graphics needed to understand them have a contrast of at least 3:1 with the colours next to them.
      **Human verification required.**
- [ ] [1.4.12 Text Spacing][1.4.12] (AA): nothing is lost or overlaps when line height is set to 1.5 times the font
      size, space after paragraphs to 2 times, letter spacing to 0.12 times and word spacing to 0.16 times.
      **Partly automated; human verification required.**
- [ ] [1.4.13 Content on Hover or Focus][1.4.13] (AA): tooltips and other content that appear on hover or focus can
      be dismissed without moving the pointer or focus, as with Escape, can themselves be hovered over, and stay until
      dismissed or no longer relevant.
      **Human verification required.**

## Operable

- [ ] [2.1.1 Keyboard][2.1.1] (A): everything can be done with the keyboard alone, without having to time keystrokes,
      unless it depends on the path of a movement, as freehand drawing does.
      **Human verification required.**
- [ ] [2.1.2 No Keyboard Trap][2.1.2] (A): focus can always be moved away from every part with the keyboard, and if
      that needs more than the arrow keys, Tab or another standard way out, the user is told how.
      **Human verification required.**
- [ ] [2.1.4 Character Key Shortcuts][2.1.4] (A): every shortcut of a single letter, digit, punctuation mark or symbol
      can be turned off or remapped, or works only while its control has focus.
      **Human verification required.**
- [ ] [2.2.1 Timing Adjustable][2.2.1] (A): each time limit can be turned off, adjusted, or extended at least ten
      times by a simple action with 20 seconds' warning, unless it is real-time, essential or over 20 hours.
      **Partly automated; human verification required.**
- [ ] [2.2.2 Pause, Stop, Hide][2.2.2] (A): moving, blinking or scrolling content that starts by itself, lasts over
      5 seconds and runs beside other content can be paused, stopped or hidden; so can content that updates itself,
      or its frequency be set.
      **Partly automated; human verification required.**
- [ ] [2.3.1 Three Flashes or Below Threshold][2.3.1] (A): nothing flashes more than three times in any second,
      unless the flashes are below the general and red flash thresholds.
      **Human verification required.**
- [ ] [2.4.1 Bypass Blocks][2.4.1] (A): blocks repeated on several pages, such as the navigation, can be skipped, as
      with a skip link, landmarks or headings.
      **Partly automated; human verification required.**
- [ ] [2.4.2 Page Titled][2.4.2] (A): every page or screen has a title that says what it is about or for.
      **Partly automated; human verification required.**
- [ ] [2.4.3 Focus Order][2.4.3] (A): keyboard focus moves through the content in an order that keeps its meaning
      and use.
      **Human verification required.**
- [ ] [2.4.4 Link Purpose (In Context)][2.4.4] (A): what a link does can be told from its text, or from its text and
      the sentence, paragraph, list item or table cell around it.
      **Partly automated; human verification required.**
- [ ] [2.4.5 Multiple Ways][2.4.5] (AA): there is more than one way to find a page, such as a menu, a search and a
      site map, except for a page that is a step in a process.
      **Human verification required.**
- [ ] [2.4.6 Headings and Labels][2.4.6] (AA): headings and labels say what their content or control is about or
      for.
      **Human verification required.**
- [ ] [2.4.7 Focus Visible][2.4.7] (AA): whatever has keyboard focus shows it visibly.
      **Human verification required.**
- [ ] [2.4.11 Focus Not Obscured (Minimum)][2.4.11] (AA): what has keyboard focus is never wholly hidden by the
      project's own content, such as a sticky header, a cookie banner or a dialog that is not modal.
      **Human verification required.**
- [ ] [2.5.1 Pointer Gestures][2.5.1] (A): whatever is done with several fingers or along a path, such as a pinch or
      a swipe, can also be done with a single tap or click, unless the gesture is essential.
      **Human verification required.**
- [ ] [2.5.2 Pointer Cancellation][2.5.2] (A): an action does not run when the pointer is pressed down: it runs on
      release, and can be cancelled by moving away first, or undone, unless acting on press is essential.
      **Human verification required.**
- [ ] [2.5.3 Label in Name][2.5.3] (A): the name that assistive technology gives a control contains the words of its
      visible label.
      **Partly automated; human verification required.**
- [ ] [2.5.4 Motion Actuation][2.5.4] (A): whatever is done by moving the device, or by a gesture toward it, can also
      be done with ordinary controls, and the response to motion can be turned off.
      **Human verification required.**
- [ ] [2.5.7 Dragging Movements][2.5.7] (AA): whatever is done by dragging can also be done with a single pointer
      without dragging, such as by clicks, unless dragging is essential.
      **Human verification required.**
- [ ] [2.5.8 Target Size (Minimum)][2.5.8] (AA): every target for a pointer is at least 24 by 24 CSS pixels, or is
      spaced so that a circle 24 pixels across, centred on it, meets no other target or circle; links within text,
      browser controls, targets with an equivalent that is big enough, and essential sizes are exempt.
      **Partly automated; human verification required.**

## Understandable

- [ ] [3.1.1 Language of Page][3.1.1] (A): the language of every page or screen is set in code, as with `lang` on
      `<html>`, and is the language it is written in.
      **Automated.**
- [ ] [3.1.2 Language of Parts][3.1.2] (AA): every passage or phrase in another language is marked with its language,
      except proper names, technical terms and words that have become part of the language around them.
      **Partly automated; human verification required.**
- [ ] [3.2.1 On Focus][3.2.1] (A): moving focus to something does not by itself change the context, such as by
      opening a window, submitting a form or moving focus elsewhere.
      **Human verification required.**
- [ ] [3.2.2 On Input][3.2.2] (A): changing a setting, such as choosing an option or ticking a box, does not change
      the context unless the user has been told beforehand that it will.
      **Human verification required.**
- [ ] [3.2.3 Consistent Navigation][3.2.3] (AA): navigation repeated on several pages comes in the same order on
      each, unless the user changes it.
      **Human verification required.**
- [ ] [3.2.4 Consistent Identification][3.2.4] (AA): parts with the same function are named and shown the same way
      on every page.
      **Human verification required.**
- [ ] [3.2.6 Consistent Help][3.2.6] (A): help repeated on several pages, such as contact details, a contact form, a
      chat or a help page, comes in the same place relative to the rest of the page on each.
      **Human verification required.**
- [ ] [3.3.1 Error Identification][3.3.1] (A): an input error that is detected is pointed out, with the field named
      and the error described in text.
      **Human verification required.**
- [ ] [3.3.2 Labels or Instructions][3.3.2] (A): every field and control that takes input has a label or
      instructions.
      **Partly automated; human verification required.**
- [ ] [3.3.3 Error Suggestion][3.3.3] (AA): when an input error is detected and a correction is known, the correction
      is suggested, unless that would undermine security or purpose.
      **Human verification required.**
- [ ] [3.3.4 Error Prevention (Legal, Financial, Data)][3.3.4] (AA): a submission that makes a legal commitment or a
      payment, changes or deletes the user's data, or submits test answers can be reversed, is checked for errors
      with a chance to correct them, or is confirmed before it is final.
      **Human verification required.**
- [ ] [3.3.7 Redundant Entry][3.3.7] (A): what the user has already entered or been given in a process is filled in
      or offered for selection, not asked for again, unless entering it again is essential, needed for security, or
      the information is no longer valid.
      **Human verification required.**
- [ ] [3.3.8 Accessible Authentication (Minimum)][3.3.8] (AA): logging in does not depend on a cognitive function
      test, such as recalling a password or solving a puzzle, unless another way is offered, the test can be passed
      with help such as a password manager or pasting, or it only asks to recognise objects or the user's own
      content.
      **Human verification required.**

## Robust

- [ ] [4.1.2 Name, Role, Value][4.1.2] (A): every control tells assistive technology its name, role and state, and
      what the user can set can also be set by assistive technology, which is told of every change.
      **Partly automated; human verification required.**
- [ ] [4.1.3 Status Messages][4.1.3] (AA): messages of a result, progress or an error that do not take focus are
      still announced by assistive technology, as with `role="status"` or `aria-live`.
      **Human verification required.**

[1.1.1]: https://www.w3.org/WAI/WCAG22/Understanding/non-text-content
[1.2.1]: https://www.w3.org/WAI/WCAG22/Understanding/audio-only-and-video-only-prerecorded
[1.2.2]: https://www.w3.org/WAI/WCAG22/Understanding/captions-prerecorded
[1.2.3]: https://www.w3.org/WAI/WCAG22/Understanding/audio-description-or-media-alternative-prerecorded
[1.2.4]: https://www.w3.org/WAI/WCAG22/Understanding/captions-live
[1.2.5]: https://www.w3.org/WAI/WCAG22/Understanding/audio-description-prerecorded
[1.3.1]: https://www.w3.org/WAI/WCAG22/Understanding/info-and-relationships
[1.3.2]: https://www.w3.org/WAI/WCAG22/Understanding/meaningful-sequence
[1.3.3]: https://www.w3.org/WAI/WCAG22/Understanding/sensory-characteristics
[1.3.4]: https://www.w3.org/WAI/WCAG22/Understanding/orientation
[1.3.5]: https://www.w3.org/WAI/WCAG22/Understanding/identify-input-purpose
[1.4.1]: https://www.w3.org/WAI/WCAG22/Understanding/use-of-color
[1.4.2]: https://www.w3.org/WAI/WCAG22/Understanding/audio-control
[1.4.3]: https://www.w3.org/WAI/WCAG22/Understanding/contrast-minimum
[1.4.4]: https://www.w3.org/WAI/WCAG22/Understanding/resize-text
[1.4.5]: https://www.w3.org/WAI/WCAG22/Understanding/images-of-text
[1.4.10]: https://www.w3.org/WAI/WCAG22/Understanding/reflow
[1.4.11]: https://www.w3.org/WAI/WCAG22/Understanding/non-text-contrast
[1.4.12]: https://www.w3.org/WAI/WCAG22/Understanding/text-spacing
[1.4.13]: https://www.w3.org/WAI/WCAG22/Understanding/content-on-hover-or-focus
[2.1.1]: https://www.w3.org/WAI/WCAG22/Understanding/keyboard
[2.1.2]: https://www.w3.org/WAI/WCAG22/Understanding/no-keyboard-trap
[2.1.4]: https://www.w3.org/WAI/WCAG22/Understanding/character-key-shortcuts
[2.2.1]: https://www.w3.org/WAI/WCAG22/Understanding/timing-adjustable
[2.2.2]: https://www.w3.org/WAI/WCAG22/Understanding/pause-stop-hide
[2.3.1]: https://www.w3.org/WAI/WCAG22/Understanding/three-flashes-or-below-threshold
[2.4.1]: https://www.w3.org/WAI/WCAG22/Understanding/bypass-blocks
[2.4.2]: https://www.w3.org/WAI/WCAG22/Understanding/page-titled
[2.4.3]: https://www.w3.org/WAI/WCAG22/Understanding/focus-order
[2.4.4]: https://www.w3.org/WAI/WCAG22/Understanding/link-purpose-in-context
[2.4.5]: https://www.w3.org/WAI/WCAG22/Understanding/multiple-ways
[2.4.6]: https://www.w3.org/WAI/WCAG22/Understanding/headings-and-labels
[2.4.7]: https://www.w3.org/WAI/WCAG22/Understanding/focus-visible
[2.4.11]: https://www.w3.org/WAI/WCAG22/Understanding/focus-not-obscured-minimum
[2.5.1]: https://www.w3.org/WAI/WCAG22/Understanding/pointer-gestures
[2.5.2]: https://www.w3.org/WAI/WCAG22/Understanding/pointer-cancellation
[2.5.3]: https://www.w3.org/WAI/WCAG22/Understanding/label-in-name
[2.5.4]: https://www.w3.org/WAI/WCAG22/Understanding/motion-actuation
[2.5.7]: https://www.w3.org/WAI/WCAG22/Understanding/dragging-movements
[2.5.8]: https://www.w3.org/WAI/WCAG22/Understanding/target-size-minimum
[3.1.1]: https://www.w3.org/WAI/WCAG22/Understanding/language-of-page
[3.1.2]: https://www.w3.org/WAI/WCAG22/Understanding/language-of-parts
[3.2.1]: https://www.w3.org/WAI/WCAG22/Understanding/on-focus
[3.2.2]: https://www.w3.org/WAI/WCAG22/Understanding/on-input
[3.2.3]: https://www.w3.org/WAI/WCAG22/Understanding/consistent-navigation
[3.2.4]: https://www.w3.org/WAI/WCAG22/Understanding/consistent-identification
[3.2.6]: https://www.w3.org/WAI/WCAG22/Understanding/consistent-help
[3.3.1]: https://www.w3.org/WAI/WCAG22/Understanding/error-identification
[3.3.2]: https://www.w3.org/WAI/WCAG22/Understanding/labels-or-instructions
[3.3.3]: https://www.w3.org/WAI/WCAG22/Understanding/error-suggestion
[3.3.4]: https://www.w3.org/WAI/WCAG22/Understanding/error-prevention-legal-financial-data
[3.3.7]: https://www.w3.org/WAI/WCAG22/Understanding/redundant-entry
[3.3.8]: https://www.w3.org/WAI/WCAG22/Understanding/accessible-authentication-minimum
[4.1.2]: https://www.w3.org/WAI/WCAG22/Understanding/name-role-value
[4.1.3]: https://www.w3.org/WAI/WCAG22/Understanding/status-messages
