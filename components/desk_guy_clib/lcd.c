#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_lcd_spd2010.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_io.h"
#include "bmp.h"
#include "esp_log.h"

esp_lcd_panel_handle_t panel_handle;

#define TAG "LCD"

#define LCD_HOST SPI2_HOST

#define PIN_CS 21
#define PIN_PCLK 40
#define PIN_D0 46
#define PIN_D1 45
#define PIN_D2 42
#define PIN_D3 41
#define PIN_BK 5

#define LCD_H_RES 412
#define LCD_V_RES 412

void lcd_init(void)
{
    ESP_LOGI(TAG, "Init backlight");
    gpio_set_direction(PIN_BK, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_BK, 1);

    ESP_LOGI(TAG, "Init SPI bus");

    const spi_bus_config_t buscfg =
        SPD2010_PANEL_BUS_QSPI_CONFIG(
            PIN_PCLK, PIN_D0, PIN_D1, PIN_D2, PIN_D3,
            4096);

    ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO));

    ESP_LOGI(TAG, "Install panel IO");

    esp_lcd_panel_io_handle_t io_handle = NULL;
    const esp_lcd_panel_io_spi_config_t io_config =
        SPD2010_PANEL_IO_QSPI_CONFIG(PIN_CS, NULL, NULL);

    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(
        (esp_lcd_spi_bus_handle_t)LCD_HOST,
        &io_config,
        &io_handle));

    ESP_LOGI(TAG, "Install LCD driver");

    spd2010_vendor_config_t vendor_config = {
        .flags = {
            .use_qspi_interface = 1,
        },
    };

    const esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = -1,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 16,
        .vendor_config = &vendor_config,
    };

    ESP_ERROR_CHECK(
        esp_lcd_new_panel_spd2010(io_handle, &panel_config, &panel_handle));

    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));
}

void draw_bmp_file(const char *name)
{
    Bitmap *img = malloc(sizeof(Bitmap));
    if (bmp_load(name, img) == 0)
    {
        ESP_LOGE(TAG, "Failed to load image");
    }
    ESP_LOGI(TAG, "Loaded Image");
    esp_lcd_panel_draw_bitmap(panel_handle, 0, 0, BMP_WIDTH, BMP_HEIGHT, img);
    free(img);
}