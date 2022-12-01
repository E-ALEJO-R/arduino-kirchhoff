#include <Arduino.h>
#include "../dependencies/uno/FreeRTOS/src/Arduino_FreeRTOS.h"
#include "../dependencies/uno/FreeRTOS/src/semphr.h"

float *V1 = new float(0.0);
float *V2 = new float(0.0);

float R1 = 27000;
float R2 = 11;
float R3 = 27;

float *I1 = new float(0.0);
float *I2 = new float(0.0);
float *I3 = new float(0.0);

SemaphoreHandle_t mutex = nullptr;

void readV1(void *pvParameters);

void readV2(void *pvParameters);

void readI(void *pvParameters);

void show(void *pvParameters);

void print(String prefix, float value, String postfix);

float getVoltage(float value);

// void gaussJordan(int rows, int columns, float pivote, float aux, float matrix[3][4]);
void gaussJordan(int rows, int columns, float pivote, float aux, float matrix[3][4]);


void setup() {
    Serial.begin(9600);
    mutex = xSemaphoreCreateMutex();
    xTaskCreate(readV1, "V1", 127, nullptr, 2, nullptr);
    xTaskCreate(readV2, "V2", 127, nullptr, 2, nullptr);
    xTaskCreate(readI, "gaussJordan", 127, nullptr, 2, nullptr);
    xTaskCreate(show, "show", 127, nullptr, 2, nullptr);
}

void loop() {
}

// para una fuente de 12 voltios.
void readV1(void *pvParameters) {
    float voltage;
    int value;
    while (true) {
        value = analogRead(A0);
        voltage = getVoltage(value);
        if (*V1 != voltage) {
            *V1 = voltage;
        }
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

// para una fuente de 5 voltios.
void readV2(void *pvParameters) {
    float voltage;
    int value;
    while (true) {
        value = analogRead(A1);
        voltage = getVoltage(value);
        if (*V2 != voltage) {
            *V2 = voltage;
        }
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

// para calcular las corrientes.
void readI(void *pvParameters) {
    int rows = 3, columns = 4;
    float aux = 0, pivote = 0;
    while (true) {
        float m[3][4] = {
                {27011, 11, 0,  *V1},
                {11,    38, 0,  *V2},
                {1,     1,  -1, 0}
        };
        gaussJordan(rows, columns, pivote, aux, m);
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

// para mostrar los datos.
void show(void *pvParameters) {
    while (true) {
        print("V1: ", *V1, "[V]");
        print("V2: ", *V2, "[V]");

        print("I1: ", *I1, "[A]");
        print("I2: ", *I2, "[A]");
        print("I3: ", *I3, "[A]");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void print(String prefix, float value, String postfix) {
    xSemaphoreTake(mutex, portMAX_DELAY);
    Serial.print(prefix);
    Serial.print(value, 5);
    Serial.println(postfix);
    xSemaphoreGive(mutex);
}

float getVoltage(float value) {
    double a = map(value, 0, 1023, 0, 25000) / 1000.0;
    return a - 0.20;
}

void gaussJordan(int rows, int columns, float pivote, float aux, float matrix[3][4]) {
    // reducción por reglones
    for (short int i = 0; i < rows; i++) {
        pivote = matrix[i][i];
        // convertir el pivote matrix uno y aplicar las operaciones sobre toda la fila.
        for (short int k = 0; k < columns; k++) {
            matrix[i][k] = matrix[i][k] / pivote;
        }
        for (short int j = 0; j < rows; j++) {
            if (i != j) { // no estoy en la diagonal
                aux = matrix[j][i];
                for (short int k = 0; k < columns; k++) { // convertir a cero toda la columna del pivote.
                    matrix[j][k] = matrix[j][k] - aux * matrix[i][k];
                }
            }
        }
    }
    *I1 = matrix[0][3];
    *I2 = matrix[1][3];
    *I3 = matrix[2][3];
}