/**
 * PROYECTO "LEY DE KIRCHHOFF"
 * Fecha: 30/11/2022
 * Descripción: desarrollado con el framework arduino RTOS.
 */

#include <Arduino.h>
#include "Arduino_FreeRTOS.h"
#include "semphr.h"
#include "Task.h"
#include "main.h"

/**
 * A continuación vamos a mostrara el código con el que fue posible realizar este experimento.
 */

// primero declaramos dos variables de tipo float que almacena valores leídos desde el pin
// ADC del arduino.
float *V1 = new float;
float *V2 = new float;


// estas son valores de la resistencia.
float R1 = 27000;
float R2 = 11;
float R3 = 27;

// variables para almacenar las corrientes calculados.
float *I1 = new float;
float *I2 = new float;
float *I3 = new float;

// para poder leer los valores simultaneamente se utilizo la libreria FreeRTOS, que es un sistema en tiempo real.
// que imprime los 5 valore atraves del monitor serial.
SemaphoreHandle_t mutex = nullptr;


// funcion para leer valores de la vuente de 12 voltios.
void readV1(void *pvParameters);

// funcion para leer valores de la fuente de 5 voltios.
void readV2(void *pvParameters);

// función para procesar los datos leidos..
void readI(void *pvParameters);


// función para mostrar valores procesados a traves de monitor serial.
void show(void *pvParameters);

// función que para imprimir valores.
void print(String prefix, float value, String postfix);

// función para leer el voltage.
float getVoltage(float value);

// función para resolver sistema de ecuaciones por el metodo de gauss jordan.
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

/**
 * Tarea para leer el valor de voltage de la fuente variable de 12[V].
 * Esta función obtiene los valores desde el pin ADC del arduino, para la fuente
 * regulable de 12 voltios.
 * @param pvParameters
 */
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

/**
 * Tarea para leer el valor de voltaje de la fuente de 5[v]
 * @param pvParameters
 */
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

/**
 * Tarea para calcular las corrientes I1, I2 e I3.
 * @param pvParameters
 */
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

/**
 * Tarea para mostrar los valores calculados mediante la ley de kirchhoff.
 * @param pvParameters
 */
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

/**
 * Función para mostrar datos a través del monitor serial.
 * @param prefix descripción.
 * @param value valor a mostrar.
 * @param postfix unidad de medida.
 */
void print(String prefix, float value, String postfix) {
    xSemaphoreTake(mutex, portMAX_DELAY);
    Serial.print(prefix);
    Serial.print(value, 5);
    Serial.println(postfix);
    xSemaphoreGive(mutex);
}

/**
 * Función para calcular el voltaje de entrada.
 * @param value valor obtenido desde el pin adc del arduino.
 * @return voltage calculado.
 */
float getVoltage(float value) {
    double a = map(value, 0, 1023, 0, 25000) / 1000.0;
    return a - 0.20;
}

/**
 * Función para resolver sistema de ecuaciones lineales mediante el método de gauss-jordan.
 *  ┌───────┬────┬────┬────┐              ┌───┬───┬───┬───┐ <br>
 *  │ 27011 │ 11 │ 0  │ 12 │              │ 1 │ 0 │ 0 │ a │ <br>
 *  ├───────┼────┼────┼────┤              ├───┼───┼───┼───┤ <br>
 *  │  11   │ 38 │ 0  │ 5  │      ->      │ 0 │ 1 │ 0 │ b │ <br>
 *  ├───────┼────┼────┼────┤              ├───┼───┼───┼───┤ <br>
 *  │  1    │ 1  │ -1 │ 0  │              │ 0 │ 0 │ 1 │ c │ <br>
 *  └───────┴────┴────┴────┘              └───┴───┴───┴───┘ <br>
 * @param rows filas de la matriz.
 * @param columns columnas de la matriz.
 * @param pivote diagonal principal.
 * @param aux tmp.
 * @param matrix matriz.
 */
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