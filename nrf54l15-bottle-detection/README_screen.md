The 1.8" 128x160 TFT uses an ST7735R controller over the MIPI-DBI SPI interface.
Zephyr has a built-in driver for it. This example renders a logo image in RGB565
format and demonstrates the drawing API — including a bounding-box overlay — that
is suitable for use with the OV7670 camera sensor output.

## Pin Wiring

`uart30` is disabled and `spi30` is used instead. On the nRF54L15, `uart30` and
`spi30` share the same peripheral address block (`0x104000`) and domain
(domain-30 / gpiote30), which is the only domain that can drive Port 0 (P0.xx)
pins. `spi22` (domain-20 / gpiote20) cannot route to Port 0, so it is disabled.
RESET is tied high in hardware via 10 kΩ — no MCU pin required.

| Display Pin | nRF54L15 DK Pin | Function     | Notes                               |
|-------------|-----------------|--------------|-------------------------------------|
| SCK         | P0.00           | SPI Clock    | spi30 (domain-30) — uart30 disabled |
| SDA/MOSI    | P0.01           | SPI Data     | spi30 (domain-30) — uart30 disabled |
| CS          | P0.02           | Chip Select  | spi30 (domain-30) — uart30 disabled |
| DC/A0       | P0.03           | Data/Command | spi30 (domain-30) — uart30 disabled |
| RESET       | 3.3V via 10 kΩ  | Hardware Reset | No MCU pin needed — tie high      |
| VCC         | 3.3V            | Power        |                                     |
| GND         | GND             | Ground       |                                     |
| LED/BL      | 3.3V            | Backlight    |                                     |

## Project Structure

```
tft-screen-1p8inch/
├── CMakeLists.txt
├── prj.conf
├── README.md
├── boards/
│   └── nrf54l15dk_nrf54l15_cpuapp.overlay   # spi30 on P0; uart30/spi22 disabled
└── src/
    ├── tft_display.h    # Public API — include this in any project
    ├── tft_display.c    # Implementation: font, primitives, image blit, bbox
    ├── usnlogo.h        # USN logo image data (RGB565, 160×128)
    └── main.c           # Demo entry point
```

## Importing into Another Project

Copy `src/tft_display.h` and `src/tft_display.c` into your project and add
`tft_display.c` to your `CMakeLists.txt`:

```cmake
target_sources(app PRIVATE src/tft_display.c)
```

Then include the header and use the API:

```c
#include "tft_display.h"

const struct device *display = TFT_DEVICE();
tft_init(display);

/* Clear screen and show a camera frame */
tft_fill_screen(display, TFT_COLOR_BLACK);
tft_draw_image(display, 0, 0, 160, 128, frame_buf);

/* Overlay a bounding box with label */
tft_draw_bounding_box(display, 10, 10, 60, 40, "cat 0.91");
```

## API Reference

### Lifecycle
| Function | Description |
|---|---|
| `tft_init(dev)` | Check device ready, turn blanking off. Returns 0 or `-ENODEV`. |

### Display constants
| Macro | Value | Description |
|---|---|---|
| `TFT_DEVICE()` | — | Get `st7735r` device from devicetree |
| `TFT_WIDTH` | 160 | Display width in pixels |
| `TFT_HEIGHT` | 128 | Display height in pixels |
| `TFT_BPP` | 2 | Bytes per pixel (RGB565) |
| `TFT_COLOR_BLACK/WHITE/RED/…` | — | Common RGB565 colour constants |

### Drawing functions
| Function | Description |
|---|---|
| `tft_fill_screen(dev, color)` | Fill entire display with one colour |
| `tft_fill_rect(dev, x, y, w, h, color)` | Fill a rectangle |
| `tft_draw_hline(dev, x, y, w, color)` | Draw a horizontal line |
| `tft_draw_vline(dev, x, y, h, color)` | Draw a vertical line |
| `tft_draw_char(dev, x, y, c, fg, bg)` | Draw a single 5×7 character |
| `tft_draw_string(dev, x, y, str, fg, bg)` | Draw a null-terminated string |
| `tft_draw_image(dev, x, y, w, h, rgb565)` | Blit a raw RGB565 frame buffer |
| `tft_draw_bounding_box(dev, x, y, w, h, label)` | CV-style labelled bounding box |

## Build & Flash

```bash
west build -b nrf54l15dk/nrf54l15/cpuapp
west flash
```
