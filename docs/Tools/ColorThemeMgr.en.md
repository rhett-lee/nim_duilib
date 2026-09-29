English | [简体中文](ColorThemeMgr.md)

> Last synced: 2026-09-29

# Principle and Usage of the "Generate New Color Scheme" Feature

"Generate New Color Scheme" is a core intelligent feature of the editor. It can quickly generate a complete set of theme colors based on a specified base color, avoiding the tedious operation of manually adjusting colors one by one. The following explains it in detail from two aspects: principle and usage.

## I. Feature Principle

This feature is based on the **HSL color model** (Hue, Saturation, Lightness) and generates a complete color scheme through three steps: "base color definition + color rule calculation + control attribute adaptation". The core logic is as follows:

### 1. Core Color Model: HSL Three-element Control

The final color presentation is determined by "Hue (H), Saturation (S), Lightness (L)". The feature generates colors by adjusting these three parameters:

- **Hue (H)**: Determines the "type" of the color (e.g., red H=0°, blue H=240°), range 0-360°;
- **Saturation (S)**: Determines the "vividness" of the color (0=gray, 1=pure color), range 0-1;
- **Lightness (L)**: Determines the "brightness" of the color (0=black, 0.5=normal, 1=white), range 0-1.

### 2. Three-step Generation Logic

#### Step 1: Parse Base Color Parameters

1. Extract the user-specified color from the "base main color" selector (default #3B82F6, i.e., blue);
2. Convert the base color from "RGB format" to "HSL format", obtaining its original Hue (H), Saturation (S), and Lightness (L) as the basis for color calculation.

#### Step 2: Generate Main Hue by Color Scheme Type

Three classic color scheme rules are provided; users can choose according to their needs. The core is to adjust the "Hue (H)" to achieve different styles:

| Color scheme type | Principle description | Applicable scenario |
|----------|----------|----------|
| Monochromatic | Keep the base color's "Hue (H)" unchanged, and only adjust the "Lightness (L)" to generate same-color shades of different depths | For a simple, unified visual style (such as office software, tool applications) |
| Complementary | Add "Hue (H) +180°" to the base color (e.g., blue→orange) to form a strong contrasting color | Scenarios that need to highlight points (such as buttons, warning prompts) |
| Analogous | Set the base color's "Hue (H) ±30°" (e.g., blue→cyan/purple) to generate harmonious and similar colors | For a rich but non-conflicting visual hierarchy (such as multi-module interfaces) |

#### Step 3: Adapt Colors by Control Attributes

After generating the main hue, automatically adjust the "Saturation (S)" and "Lightness (L)" according to the functional attributes of the controls (such as text, background, border) to ensure the color scheme conforms to visual logic:

- **Text/font**: Reduce the magnitude of lightness adjustment (lightness step ÷2) to avoid difficulty reading due to being too bright or too dark;
- **Background/border (bg/border)**: The original logic was "set saturation to 0 (gray scale)". After modification, the base color's hue is retained, only reducing saturation (e.g., 15% of the base color's saturation) to ensure softness without grabbing focus;
- **Functional colors (blue/green/red)**: Retain the saturation of the main hue, only fine-tune the lightness, to ensure functional elements such as buttons and prompts are eye-catching.

### 3. The Role of the Lightness Step

The "Lightness step" (default 15, range 5-30) controls the "magnitude" of the brightness difference of colors:

- Larger step: the generated colors have a stronger light-dark contrast (e.g., bright background → dark text);
- Smaller step: the transition of generated colors is smoother (e.g., smaller differences in background colors of different modules);
- Calculation logic: automatically determine the adjustment direction based on the lightness (L) of the original color (if L>0.5, reduce lightness; if L<0.5, increase lightness), ensuring the color is always clearly visible.

## II. Feature Usage

### 1. Operation Steps (completed in 3 steps)

#### Step 1: Preparation

1. Open the editor, and load the theme file to be modified via the "Open XML Theme File" button;
2. Click the "Generate New Color Scheme" button at the top to expand the color scheme generation panel (hidden by default).

#### Step 2: Set Color Scheme Parameters

In the generation panel, configure the 3 core parameters according to your needs:

| Parameter name | Operation method | Example setting |
|----------|----------|----------|
| Base main color | Click the color selector and choose the desired base color from the palette (manual input of 6-digit RGB value is supported, e.g., #FF5733) | Want a blue theme → choose #1E90FF; want a red theme → choose #E74C3C |
| Color scheme type | Select from the dropdown box, corresponding to different styles:<br>- Monochromatic: shades of the same color<br>- Complementary: strong contrast<br>- Analogous: harmonious transition | For office software → choose "Monochromatic"; for game interfaces → choose "Complementary" |
| Lightness step | Enter a number (5-30), or adjust via the up/down arrows | Want soft transition → set to 10; want vivid contrast → set to 25 |

#### Step 3: Apply Color Scheme

Click the "Apply Color Scheme" button at the bottom of the panel; the system will automatically complete:

1. Calculate and update all color values (converted to uppercase ARGB format, e.g., #FFFF0000);
2. Refresh the color preview on the interface in real time (current value and color block are both updated synchronously);
3. Pop up a prompt "Color scheme has been generated and applied" to confirm successful operation.

### 2. Suggestions for Subsequent Operations

- **Confirm effect**: After applying, check the colors of each control category (such as Button, Text) to confirm whether they meet expectations;
- **Manual fine-tuning**: If you are not satisfied with an individual color (e.g., a button color is too dark), you can manually modify it via the color card's "Color selector" or "Input box";
- **Save scheme**: After confirming all colors are correct, click the "Save modifications" button at the top to download the updated XML file (the file name contains the "_modified" suffix for easy distinction from the original file).

### 3. Common Problem Solutions

| Problem phenomenon | Cause and solution |
|----------|----------------|
| Some colors unchanged after applying | Cause: the original value format of that color is abnormal (not 8-digit ARGB);<br>Solution: manually modify that color, or reload the original XML file |
| Overall color scheme too dark/too bright | Cause: improper lightness step setting, or abnormal base color lightness;<br>Solution: adjust the "Lightness step" (too dark → reduce step, too bright → increase step), or change the base color |
| Complementary color contrast too harsh | Cause: the complementary color itself has high contrast, and the saturation is not reduced;<br>Solution: choose "Analogous", or reduce the saturation of the base color (need to manually adjust the base color's RGB value, e.g., #FF5733→#FF8A65) |

## III. Summary of Feature Advantages

1. **Efficiency**: Generate a complete theme in 1 minute, replacing the tedious operation of manually adjusting dozens of colors;
2. **Normativity**: Based on the HSL model and control attribute adaptation, ensuring the color scheme conforms to visual design logic (e.g., text not harsh, background not grabbing focus);
3. **Flexibility**: Supports 3 color scheme types + lightness step adjustment, meeting the visual needs of different scenarios (office, games, tools);
4. **Compatibility**: The generated color values are automatically converted to uppercase ARGB format, fully matching the format requirements of XML theme files, with no manual conversion needed.
