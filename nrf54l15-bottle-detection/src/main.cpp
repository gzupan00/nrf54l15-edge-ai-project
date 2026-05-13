/*
 * OV7670 + FIFO Camera Demo — nRF54L15 DK (Zephyr)
 * Button-triggered Edge Impulse inference.
 */

#include "edge-impulse-sdk/classifier/ei_run_classifier.h"
#include "model-parameters/model_metadata.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>

extern "C" {
#include "ov7670.h"
#include "fifo.h"
#include "tft_display.h"
}

LOG_MODULE_REGISTER(app, LOG_LEVEL_INF);

#define BUTTON_PORT_NODE DT_NODELABEL(gpio1)
#define BUTTON_PIN 9   // P1.09 = Button1/SW1

static const struct device *button_gpio = DEVICE_DT_GET(BUTTON_PORT_NODE);
static const struct device *display;

static uint8_t frame_buf[IMG_SIZE] __aligned(4);

static int ei_get_data(size_t offset, size_t length, float *out_ptr)
{
    for (size_t i = 0; i < length; i++) {
        size_t input_index = offset + i;

        int x50 = input_index % 50;
        int y50 = input_index / 50;

        int src_x = (x50 * IMG_W) / 50;
        int src_y = (y50 * IMG_H) / 50;

        int src_index = (src_y * IMG_W + src_x) * 2;

        uint16_t p =
            ((uint16_t)frame_buf[src_index] << 8) |
            frame_buf[src_index + 1];

        uint8_t r = ((p >> 11) & 0x1F) << 3;
        uint8_t g = ((p >> 5) & 0x3F) << 2;
        uint8_t b = (p & 0x1F) << 3;

        float gray = (0.299f * r) + (0.587f * g) + (0.114f * b);

        out_ptr[i] = gray;
    }

    return 0;
}

static void draw_box_on_frame(int x, int y, int w, int h)
{
    // Edge Impulse box is on 50x50 image.
    // Convert box coords to camera 160x120 image.
    int x0 = (x * IMG_W) / EI_CLASSIFIER_INPUT_WIDTH;
    int y0 = (y * IMG_H) / EI_CLASSIFIER_INPUT_HEIGHT;
    int x1 = ((x + w) * IMG_W) / EI_CLASSIFIER_INPUT_WIDTH;
    int y1 = ((y + h) * IMG_H) / EI_CLASSIFIER_INPUT_HEIGHT;

    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 >= IMG_W) x1 = IMG_W - 1;
    if (y1 >= IMG_H) y1 = IMG_H - 1;

    // RGB565 red
    uint16_t color = 0xF800;

    for (int xx = x0; xx <= x1; xx++) {
        int top = (y0 * IMG_W + xx) * 2;
        int bottom = (y1 * IMG_W + xx) * 2;

        frame_buf[top] = color >> 8;
        frame_buf[top + 1] = color & 0xFF;

        frame_buf[bottom] = color >> 8;
        frame_buf[bottom + 1] = color & 0xFF;
    }

    for (int yy = y0; yy <= y1; yy++) {
        int left = (yy * IMG_W + x0) * 2;
        int right = (yy * IMG_W + x1) * 2;

        frame_buf[left] = color >> 8;
        frame_buf[left + 1] = color & 0xFF;

        frame_buf[right] = color >> 8;
        frame_buf[right + 1] = color & 0xFF;
    }
}

static void run_bottle_inference(void)
{
    printk("Button pressed - running inference...\n");

    signal_t signal;
    signal.total_length =
        EI_CLASSIFIER_INPUT_WIDTH *
        EI_CLASSIFIER_INPUT_HEIGHT;

    signal.get_data = &ei_get_data;

    ei_impulse_result_t result = {0};

    EI_IMPULSE_ERROR err =
        run_classifier(&signal, &result, false);

    printk("Inference done err=%d boxes=%d\n",
           err,
           result.bounding_boxes_count);

    if (err != EI_IMPULSE_OK) {
        printk("Inference failed\n");
        return;
    }

    for (size_t i = 0; i < result.bounding_boxes_count; i++) {
        ei_impulse_result_bounding_box_t bb =
            result.bounding_boxes[i];

        int confidence = (int)(bb.value * 100.0f);

printk("BB label=%s value=%d%% x=%d y=%d w=%d h=%d\n",
       bb.label,
       confidence,
       bb.x,
       bb.y,
       bb.width,
       bb.height);

if (bb.value > 0.20f) {
    printk("Bottle detected: %d%%\n", confidence);
    draw_box_on_frame(bb.x, bb.y, bb.width, bb.height);
    tft_draw_image(display, 0, 4, IMG_W, IMG_H, frame_buf);

}
    }
run_classifier_deinit();
k_msleep(100);
run_classifier_init();
}

int main(void)
{
    printk("\n*** OV7670 FIFO Camera + Edge Impulse Demo ***\n");

    display = TFT_DEVICE();

    if (tft_init(display) != 0) {
        printk("Display not ready\n");
        return -ENODEV;
    }

    tft_fill_screen(display, TFT_COLOR_BLACK);

    if (ov7670_init() != 0) {
        printk("OV7670 init failed\n");
        return -EIO;
    }

    if (fifo_init() != 0) {
        printk("FIFO init failed\n");
        return -EIO;
    }

    if (!device_is_ready(button_gpio)) {
        printk("Button GPIO not ready\n");
        return -ENODEV;
    }

    gpio_pin_configure(button_gpio, BUTTON_PIN, GPIO_INPUT | GPIO_PULL_UP);

    run_classifier_init();

    k_msleep(300);

    LOG_INF("Ready — camera %dx%d RGB565 (%u bytes/frame)",
            IMG_W, IMG_H, (unsigned)IMG_SIZE);

    printk("Press Button 1 to run inference.\n");

    bool last_pressed = false;

    for (uint32_t n = 1; ; n++) {
        fifo_capture(frame_buf, IMG_SIZE);

        // Keep live TFT preview. If RAM/crash problems continue, comment this line.
        tft_draw_image(display, 0, 4, IMG_W, IMG_H, frame_buf);

        bool pressed = gpio_pin_get_raw(button_gpio, BUTTON_PIN) == 0;

        if (pressed && !last_pressed) {
            run_bottle_inference();
        }

        last_pressed = pressed;

        if ((n % 30) == 0) {
            LOG_INF("Frame #%u displayed", n);
        }

        k_msleep(100);
    }

    return 0;
}