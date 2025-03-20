#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "mqtt_client.h"
#include "lora.h"
#include "driver/i2c_master.h"

// Déclaration des fonctions pour éviter les erreurs de compilation
void task_lora_transmit(void *pvParameters);
void task_lora_receive(void *pvParameters);
void i2c_master_init(i2c_master_bus_handle_t *bus_handle, i2c_master_dev_handle_t *dev_handle);
esp_err_t mpu9250_register_read(i2c_master_dev_handle_t dev_handle, uint8_t reg_addr, uint8_t *data, size_t len);
esp_err_t mpu9250_register_write_byte(i2c_master_dev_handle_t dev_handle, uint8_t reg_addr, uint8_t data);

// Configuration Wi-Fi
#define WIFI_SSID "iPhone"
#define WIFI_PASS "yoan1234"

static EventGroupHandle_t s_wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0

static const char *TAG = "WiFi";
static const char *MQTT_TAG = "MQTT";
static const char *LORA_TAG = "LoRa";
static const char *I2C_TAG = "I2C";

esp_mqtt_client_handle_t client = NULL;
static char last_mqtt_message[256]; // Buffer pour stocker le message MQTT reçu

// 📡 **Configuration I2C**
#define I2C_MASTER_SCL_IO           22
#define I2C_MASTER_SDA_IO           21
#define I2C_MASTER_NUM              I2C_NUM_0
#define I2C_MASTER_FREQ_HZ          100000
#define I2C_MASTER_TX_BUF_DISABLE   0
#define I2C_MASTER_RX_BUF_DISABLE   0
#define I2C_MASTER_TIMEOUT_MS       1000

#define MPU9250_SENSOR_ADDR         0x68
#define MPU9250_WHO_AM_I_REG_ADDR   0x75
#define MPU9250_PWR_MGMT_1_REG_ADDR 0x6B
#define MPU9250_RESET_BIT           7

i2c_master_bus_handle_t bus_handle;
i2c_master_dev_handle_t dev_handle;

// 📡 **Initialisation I2C**
void i2c_master_init(i2c_master_bus_handle_t *bus_handle, i2c_master_dev_handle_t *dev_handle) {
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_MASTER_NUM,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, bus_handle));

    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = MPU9250_SENSOR_ADDR,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(*bus_handle, &dev_config, dev_handle));

    ESP_LOGI(I2C_TAG, "✅ I2C initialisé avec succès !");
}

// 📡 **Lecture d'un registre du MPU9250**
esp_err_t mpu9250_register_read(i2c_master_dev_handle_t dev_handle, uint8_t reg_addr, uint8_t *data, size_t len) {
    return i2c_master_transmit_receive(dev_handle, &reg_addr, 1, data, len, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}

// 📡 **Écriture dans un registre du MPU9250**
esp_err_t mpu9250_register_write_byte(i2c_master_dev_handle_t dev_handle, uint8_t reg_addr, uint8_t data) {
    uint8_t write_buf[2] = {reg_addr, data};
    return i2c_master_transmit(dev_handle, write_buf, sizeof(write_buf), I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}

// 📡 **Lecture du capteur I2C**
void task_read_i2c(void *pvParameters) {
    uint8_t data;
    while (1) {
        if (mpu9250_register_read(dev_handle, MPU9250_WHO_AM_I_REG_ADDR, &data, 1) == ESP_OK) {
            ESP_LOGI(I2C_TAG, "📡 MPU9250 WHO_AM_I = 0x%X", data);
        } else {
            ESP_LOGE(I2C_TAG, "❌ Erreur de lecture du MPU9250 !");
        }
        vTaskDelay(pdMS_TO_TICKS(5000)); // Lecture toutes les 5 secondes
    }
}

// 📡 **Démarrage principal**
void app_main() {
    ESP_LOGI("APP", "💡 Démarrage de l'application...");

    wifi_init();
    mqtt_app_start();
    lora_init_module();
    i2c_master_init(&bus_handle, &dev_handle);

    // Lancer la tâche de lecture I2C
    xTaskCreate(&task_read_i2c, "task_read_i2c", 4096, NULL, 5, NULL);
}
