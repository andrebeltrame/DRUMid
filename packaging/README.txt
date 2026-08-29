NOWHR DYNAMICS
DRUMid @VERSION@ — macOS (VST3, universal arm64 + x86_64)
=============================================================

MIDI drum pattern generator for melodic house, organic house, afro
house, indie dance, melodic techno and techno.

DRUMid makes no sound of its own. It generates drum patterns and hands
you the MIDI — your racks and samplers do the rest.

Pick your lanes, hit GENERATE, drag the MIDI where you want it.


INSTALLATION
------------

1. Copy DRUMid.vst3 to:

       ~/Library/Audio/Plug-Ins/VST3/

   (In Finder: Go > Go to Folder, then paste the path above.)

2. IMPORTANT — macOS quarantines anything that arrives inside a
   downloaded .zip. Open Terminal and run:

       xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/VST3/DRUMid.vst3

3. In Ableton Live: Preferences > Plug-Ins > Rescan.


USING IT IN ABLETON
-------------------

One instrument per track (the default):

   Every lane sits on C3, and dragging DRAG KIT MIDI into the Session
   View writes one MIDI track per lane — Live creates a track per
   instrument, already named Kick, Clap, Tom and so on. To export a
   single lane, drag its icon.

Driving a Drum Rack instead:

   Switch the note map to "GM / Drum Rack". Each lane then gets its own
   note and the kit drag writes a single track, so it lands as one clip
   that plays the whole rack.

To hear it live rather than exporting:

   DRUMid is an instrument that outputs MIDI, so it goes on its own MIDI
   track. On the track with your rack, set MIDI From to the DRUMid track,
   then pick DRUMid in the sub-menu, and set Monitor to In.


THE CONTROLS
------------

   GENERATE    rerolls the patterns under your current settings
   SURPRISE    rerolls the settings too — genre, energy, complexity, feel

   ENERGY      how busy the kit is, as one budget shared across lanes
   COMPLEX     syncopation, ghost notes, ratchets
   SWING       50% is straight; each genre starts at its own natural feel
   TIMING      timing humanisation
   DYNAMICS    velocity humanisation

   Locked lanes and the bar count survive both buttons.


THE GRID
--------

   click / drag        paint steps on and off
   alt + drag          velocity
   double click        cycle the ratchet
   lane name           enable / disable the lane
   drag the lane icon  export that lane alone as MIDI
   L                   lock — GENERATE will not touch this lane
   R                   reroll just this lane
   drag the note       retune the lane

There is no mute or solo: every lane drives its own instrument, so
enable/disable is the only switch that means anything here.


REQUIREMENTS
------------

macOS 10.15 or later, Apple Silicon or Intel. Any VST3 host.


Built with JUCE.
