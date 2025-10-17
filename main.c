#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_task_wdt.h"

QueueHandle_t fila = NULL;

volatile bool task1_ok = false;
volatile bool task2_ok = false;

void Task1(void *pv)
{
    esp_task_wdt_add(NULL); // registra Task1 no WDT
    int value = 0;
    while(1)
    {
        if(xQueueSend(fila, &value, 0) != pdTRUE)
        {
            Serial.printf("{Pedro Henrique - RM:88783}  - [FILA CHEIA] Não foi possível enviar valor %d\n", value);
        }
        else
        {
            Serial.printf("{Pedro Henrique - RM:88783}  - [FILA OK] Valor %d enviado para a fila\n", value);
        }
        value++;
        task1_ok = true;
        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void Task2(void *pv)
{
    esp_task_wdt_add(NULL); 
    int value = 0;
    int timeout = 0;

    while(1)
    {
        if(xQueueReceive(fila, &value, 0) == pdTRUE)
        {
            timeout = 0;
            Serial.printf("{Pedro Henrique - RM:88783}  - [FILA OK] Valor recebido: %d\n", value);
        }
        else
        {
            timeout++;
            if(timeout == 10)
                Serial.println("{Pedro Henrique - RM:88783}  - [TIMEOUT] Nenhum dado recebido nos últimos 5 segundos");
            else if(timeout == 20)
            {
                Serial.println("{Pedro Henrique - RM:88783}  - [RECUPERAÇÃO] Limpando fila");
                xQueueReset(fila);
            }
            else if(timeout == 30)
            {
                Serial.println("{Pedro Henrique - RM:88783}  - [RECUPERAÇÃO] Reiniciando o sistema");
                esp_restart();
            }
        }

        task2_ok = true;
        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void Task3(void *pv)
{
    esp_task_wdt_add(NULL); 
    while(1)
    {
        Serial.println("\n===== [SUPERVISÃO DO SISTEMA] =====");
        Serial.printf("[TASK1]: %s\n", task1_ok ? "{Pedro Henrique - RM:88783}  - Funcionando normalmente!" : "{Pedro Henrique - RM:88783}  - NÃO ESTÁ RESPONDENDO!");
        Serial.printf("[TASK2]: %s\n", task2_ok ? "{Pedro Henrique - RM:88783}  - Funcionando normalmente!" : "{Pedro Henrique - RM:88783}  - NÃO ESTÁ RESPONDENDO!");
        Serial.println("===================================");

        task1_ok = false;
        task2_ok = false;

        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void setup() 
{
    Serial.begin(115200);
    while(!Serial) { delay(10); }
    Serial.println("Iniciando sistema...");

    esp_task_wdt_config_t wdt = {
        .timeout_ms = 5000,
        .idle_core_mask = (1 << 0) | (1 << 1),
        .trigger_panic = true
    };
    esp_task_wdt_init(&wdt);

    fila = xQueueCreate(1, sizeof(int));
    if(fila == NULL)
    {
        Serial.println("{Pedro Henrique - RM:88783}  - Falha na criação da fila");
        esp_restart();
    }

    xTaskCreate(Task1, "Task1", 8192, NULL, 5, NULL);
    xTaskCreate(Task2, "Task2", 8192, NULL, 5, NULL);
    xTaskCreate(Task3, "Task3", 8192, NULL, 5, NULL);
}

void loop() 
{
}