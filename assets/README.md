# Vectoria Assets

This directory contains game assets.

## Directory Structure

```
assets/
├── sprites/      - 2D textures and sprites
├── sounds/       - Sound effects (.wav)
├── music/        - Background music (.ogg)
├── levels/       - Level definition files
└── fonts/        - Font files (optional)
```

## Required Assets

For a complete build, add the following audio files:

### Sound Effects (assets/sounds/)
- collision.wav - Object collision sound
- goal.wav - Goal reached celebration
- swipe.wav - Gravity swipe gesture
- chain.wav - Chain reaction trigger
- complete.wav - Level complete fanfare
- star.wav - Star earned sound
- click.wav - UI button click
- select.wav - Menu selection
- warning.wav - Time warning
- gameover.wav - Game over sound
- energy.wav - Energy transfer
- bounce.wav - Wall bounce

### Music (assets/sounds/)
- menu.ogg - Menu background music
- gameplay.ogg - In-game music

## Asset Generation

The game will run without audio assets (silent mode).
Visual elements are procedurally generated.
