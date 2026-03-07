#include "bmp.h"
#include "string.h"
#include <esp_log.h>

#pragma pack(push, 1)

#define BMP_TAG "bmp"

typedef struct
{
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
} BITMAPFILEHEADER;

typedef struct
{
    uint32_t biSize;
    int32_t biWidth;
    int32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t biXPelsPerMeter;
    int32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
} BITMAPINFOHEADER;

#pragma pack(pop)

static int validate_headers(const BITMAPFILEHEADER *file,
                            const BITMAPINFOHEADER *info)
{
    if (file->bfType != 0x4D42)
    {
        ESP_LOGE(BMP_TAG, "Invalid Type: %X", file->bfType);
        return 0; // 'BM'
    }
    if (info->biSize != 40)
    {
        ESP_LOGE(BMP_TAG, "Invalid header size: %X", (int)info->biSize);
        return 0; // BITMAPINFOHEADER
    }
    if (info->biWidth != BMP_WIDTH)
    {
        ESP_LOGE(BMP_TAG, "Invalid Width: %X", (int)info->biWidth);
        return 0;
    }
    if (info->biHeight != BMP_HEIGHT &&
        info->biHeight != -BMP_HEIGHT)
    {
        ESP_LOGE(BMP_TAG, "Invalid Height: %X", (int)info->biHeight);
        return 0; // allow top-down
    }
    if (info->biPlanes != 1)
    {
        ESP_LOGE(BMP_TAG, "Invalid Planes: %X", info->biPlanes);
        return 0;
    }
    if (info->biBitCount != 16)
    {
        ESP_LOGE(BMP_TAG, "Invalid Bitcount: %X", info->biBitCount);
        return 0;
    }
    if (info->biCompression != 3)
    {
        ESP_LOGE(BMP_TAG, "Invalid Compression: %X", (int)info->biCompression);
        return 0; // BI_RGB
    }
    return 1;
}

int bmp_load(const char *filename, Bitmap *out)
{
    char filepath[64] = "/sdcard/";
    strcat(filepath, filename);
    ESP_LOGI(BMP_TAG, "Loading image %s", filepath);
    FILE *f = fopen(filepath, "rb");
    if (!f)
    {
        ESP_LOGE(BMP_TAG, "Failed to open file");
        return 0;
    }

    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER infoHeader;

    if (fread(&fileHeader, sizeof(fileHeader), 1, f) != 1)
    {
        ESP_LOGE(BMP_TAG, "Failed to read file header");
        fclose(f);
        return 0;
    }

    if (fread(&infoHeader, sizeof(infoHeader), 1, f) != 1)
    {
        ESP_LOGE(BMP_TAG, "Failed to read info header");
        fclose(f);
        return 0;
    }

    if (!validate_headers(&fileHeader, &infoHeader))
    {
        ESP_LOGE(BMP_TAG, "Failed to validate headers");
        fclose(f);
        return 0;
    }

    if (fseek(f, fileHeader.bfOffBits, SEEK_SET) != 0)
    {
        ESP_LOGE(BMP_TAG, "Failed to seek offset");
        fclose(f);
        return 0;
    }

    int topDown = (infoHeader.biHeight < 0);

    // Each row is padded to 4-byte boundary
    const uint32_t rowSize = (BMP_WIDTH * 2 + 3) & ~3;
    uint8_t rowBuffer[rowSize];

    for (int y = 0; y < BMP_HEIGHT; y++)
    {

        int targetY = topDown ? y : (BMP_HEIGHT - 1 - y);

        if (fread(rowBuffer, 1, rowSize, f) != rowSize)
        {
            ESP_LOGE(BMP_TAG, "Failed to read row");
            fclose(f);
            return 0;
        }

        for (int x = 0; x < BMP_WIDTH; x++)
        {
            out->pixels[targetY * BMP_WIDTH + x] = (uint16_t)rowBuffer[2 * x] << 8 | (uint16_t)rowBuffer[2 * x + 1];
        }
    }

    fclose(f);
    return 1;
}