# GravityPaint - App Icon Specifications

## Icon Design Concept

**Visual Elements:**
- A stylized paint brush or finger drawing a curved gravity stroke
- Objects (circles/shapes) being pulled along the stroke
- Gradient background (deep purple to blue, matching game theme)
- Clean, minimal design that reads well at small sizes

**Color Palette:**
- Primary: Deep Purple (#2D1B4E) to Blue (#1A237E) gradient
- Accent: Cyan/Teal (#00BCD4) for gravity strokes
- Highlight: White or light cyan for objects/sparkles

**Style:**
- Flat design with subtle gradients
- No text in icon (app name shown separately)
- Rounded corners applied by OS

---

## Required Icon Sizes

### iOS (App Store Connect)

| Size | Scale | Pixels | Usage |
|------|-------|--------|-------|
| 1024x1024 | 1x | 1024x1024 | App Store |
| 180x180 | 3x | 60x60 | iPhone App Icon |
| 120x120 | 2x | 60x60 | iPhone App Icon |
| 167x167 | 2x | 83.5x83.5 | iPad Pro App Icon |
| 152x152 | 2x | 76x76 | iPad App Icon |
| 120x120 | 2x | 40x40 | Spotlight |
| 80x80 | 2x | 40x40 | Spotlight |
| 87x87 | 3x | 29x29 | Settings |
| 58x58 | 2x | 29x29 | Settings |
| 60x60 | 3x | 20x20 | Notification |
| 40x40 | 2x | 20x20 | Notification |

**iOS Icon Requirements:**
- PNG format
- No transparency (use solid background)
- No rounded corners (iOS applies automatically)
- sRGB color space

### Android (Google Play Console)

| Size | Usage |
|------|-------|
| 512x512 | Google Play Store listing |
| 192x192 | xxxhdpi launcher |
| 144x144 | xxhdpi launcher |
| 96x96 | xhdpi launcher |
| 72x72 | hdpi launcher |
| 48x48 | mdpi launcher |

**Android Adaptive Icon:**
- Foreground layer: 108x108 dp (432x432 px for xxxhdpi)
- Background layer: 108x108 dp (solid color or gradient)
- Safe zone: 66x66 dp centered (content should fit here)

**Android Icon Requirements:**
- PNG format (32-bit with alpha)
- Provide both legacy and adaptive icon assets

### Feature Graphic (Android)
- Size: 1024x500 pixels
- Shows app name, tagline, and gameplay preview
- Used in Google Play Store listing

---

## Screenshot Specifications

### iOS Screenshots

| Device | Size (Portrait) | Size (Landscape) |
|--------|-----------------|------------------|
| iPhone 6.7" | 1290x2796 | 2796x1290 |
| iPhone 6.5" | 1284x2778 | 2778x1284 |
| iPhone 5.5" | 1242x2208 | 2208x1242 |
| iPad Pro 12.9" | 2048x2732 | 2732x2048 |

**Required:** Up to 10 screenshots per device size

### Android Screenshots
- Minimum: 2 screenshots
- Maximum: 8 screenshots
- Size: 16:9 or 9:16 aspect ratio
- Minimum dimension: 320px
- Maximum dimension: 3840px
- PNG or JPEG format

---

## Screenshot Content Suggestions

1. **Main Menu** - Show game title and clean UI
2. **Gameplay - Drawing** - Finger drawing a gravity stroke
3. **Gameplay - Physics** - Objects responding to gravity
4. **Level Complete** - Star rating celebration
5. **Level Select** - Show progression grid
6. **Settings** - Customization options
7. **Campaign Mode** - Multiple level preview
8. **Endless Mode** - Infinite puzzle showcase

**Screenshot Tips:**
- Use actual gameplay footage
- Add brief text overlays explaining features
- Maintain consistent visual style across all screenshots
- Show the most impressive/unique features first

---

## File Checklist

### iOS Assets Needed:
- [ ] AppIcon.appiconset (all sizes)
- [ ] 6.7" iPhone screenshots (up to 10)
- [ ] 6.5" iPhone screenshots (up to 10)
- [ ] 5.5" iPhone screenshots (up to 10)
- [ ] 12.9" iPad screenshots (up to 10)

### Android Assets Needed:
- [ ] ic_launcher.png (all densities)
- [ ] ic_launcher_foreground.png (adaptive)
- [ ] ic_launcher_background.png (adaptive)
- [ ] feature_graphic.png (1024x500)
- [ ] screenshots (2-8 images)

---

## Tools for Creating Icons

**Free:**
- Figma (figma.com) - Design tool
- GIMP - Image editing
- Inkscape - Vector graphics
- Android Asset Studio (online)
- App Icon Generator (appicon.co)

**Paid:**
- Adobe Illustrator
- Adobe Photoshop
- Sketch (Mac)
- Affinity Designer
