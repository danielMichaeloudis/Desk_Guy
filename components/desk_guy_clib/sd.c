#include "sd.h"
#include <dirent.h>
#include "driver/sdmmc_host.h"
#include "esp_vfs_fat.h"
#include "string.h"

#define MOUNT_POINT "/sdcard"
static const char *SD_TAG = "SD";
uint32_t Flash_Size = 0;

#define CONFIG_EXAMPLE_PIN_CLK 14
#define CONFIG_EXAMPLE_PIN_CMD 17
#define CONFIG_EXAMPLE_PIN_D0 16
#define CONFIG_EXAMPLE_PIN_D1 -1
#define CONFIG_EXAMPLE_PIN_D2 -1
#define CONFIG_EXAMPLE_PIN_D3 -1

esp_err_t sd_init(void)
{
    esp_err_t ret;
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false, .max_files = 5, .allocation_unit_size = 16 * 1024};
    sdmmc_card_t *card;
    const char mount_point[] = MOUNT_POINT;
    ESP_LOGI(SD_TAG, "Initializing SD card");
    ESP_LOGI(SD_TAG, "Using SPI peripheral");
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    slot_config.width = 1;
    slot_config.clk = CONFIG_EXAMPLE_PIN_CLK;
    slot_config.cmd = CONFIG_EXAMPLE_PIN_CMD;
    slot_config.d0 = CONFIG_EXAMPLE_PIN_D0;
    slot_config.d1 = CONFIG_EXAMPLE_PIN_D1;
    slot_config.d2 = CONFIG_EXAMPLE_PIN_D2;
    slot_config.d3 = CONFIG_EXAMPLE_PIN_D3;
    slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;
    ESP_LOGI(SD_TAG, "Mounting filesystem");
    ret = esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot_config, &mount_config, &card);
    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            ESP_LOGE(SD_TAG, "Failed to mount filesystem. "
                             "If you want the card to be formatted, set the CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        }
        else
        {
            ESP_LOGE(SD_TAG, "Failed to initialize the card (%s). "
                             "Make sure SD card lines have pull-up resistors in place.",
                     esp_err_to_name(ret));
        }
        return ret;
    }
    ESP_LOGI(SD_TAG, "Filesystem mounted");
    return ESP_OK;
}

bool check_exists(const char *name)
{
    char filepath[64] = MOUNT_POINT;
    strcat(filepath, "/");
    strcat(filepath, name);
    FILE *fptr = fopen(filepath, "r");
    if (fptr == NULL)
    {
        ESP_LOGE(SD_TAG, "File '%s' does not exits", filepath);
        return false;
    }
    fclose(fptr);
    ESP_LOGI(SD_TAG, "File '%s' Exists", filepath);
    return true;
}

void list_fs(void)
{
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(MOUNT_POINT)) != NULL)
    { /* print all the files and directories within directory */
        while ((ent = readdir(dir)) != NULL)
        {
            ESP_LOGI(SD_TAG, "%s\n", ent->d_name);
        }
        closedir(dir);
    }
    else
    { /* could not open directory */
        ESP_LOGE(SD_TAG, "Failed to open directory");
        return;
    }
}