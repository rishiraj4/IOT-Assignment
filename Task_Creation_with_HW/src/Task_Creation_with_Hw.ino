/*
   Basic Multi Threading Arduino Example
   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <Arduino.h>

#if CONFIG_FREERTOS_UNICORE
#define TASK_RUNNING_CORE 0
#else
#define TASK_RUNNING_CORE 1
#endif

#define ANALOG_INPUT_PIN A0

#ifndef LED_BUILTIN
#define LED_BUILTIN 13  // Specify the pin your LED is connected to
#endif

// Define two tasks for Blink & AnalogRead.
void TaskBlink(void *pvParameters);
void TaskAnalogRead(void *pvParameters);
TaskHandle_t analog_read_task_handle;  // You can (don't have to) use this to manipulate a task from elsewhere.

// The setup function runs once when you press reset or power on the board.
void setup() {
    // Initialize serial communication at 115200 bits per second:
    Serial.begin(115200);

    // Set up two tasks to run independently.
    xTaskCreate(
        TaskBlink, "Task Blink", 2048,  // Stack size
        NULL,  // No parameter required for this task
        1,  // Priority
        NULL  // Task handle is not used here - simply pass NULL
    );

    // This variant of task creation can also specify on which core it will be run (only relevant for multi-core ESPs)
    xTaskCreatePinnedToCore(
        TaskAnalogRead, "Analog Read", 2048,  // Stack size
        NULL,  // When no parameter is used, simply pass NULL
        5,  // Priority
        &analog_read_task_handle,  // With task handle we will be able to manipulate with this task.
        0 // Core on which the task will run
    );

    Serial.println("Basic Multi Threading Arduino Example");
    Serial.print("Loop Core ID: ");
    Serial.println(xPortGetCoreID());
}

void loop() {
    // No need to delete tasks here. Let them run continuously.
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

void TaskBlink(void *pvParameters) {  // This is a task.
    pinMode(LED_BUILTIN, OUTPUT);  // Initialize digital LED_BUILTIN on pin 13 as an output.
    Serial.println("TaskBlink created");
    Serial.print("TaskBlink Core ID: ");
    Serial.println(xPortGetCoreID());

    for (;;) {  // A Task shall never return or exit.
        // Dynamically read the potentiometer value
        int sensorValue = analogRead(ANALOG_INPUT_PIN);
        // Map the potentiometer value to a range of 100ms to 1000ms
        int blink_delay = map(sensorValue, 0, 4095, 100, 1000);

        digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED on
        vTaskDelay(pdMS_TO_TICKS(blink_delay));  // Non-blocking delay (FreeRTOS)
        digitalWrite(LED_BUILTIN, LOW);   // Turn the LED off
        vTaskDelay(pdMS_TO_TICKS(blink_delay));  // Non-blocking delay (FreeRTOS)

        // Print the delay for debugging
        Serial.print("Blink Delay: ");
        Serial.println(blink_delay);
    }
}

void TaskAnalogRead(void *pvParameters) {  // This is a task.
    Serial.println("TaskAnalogRead created");

    // Check if the given analog pin is usable - if not - delete this task
    if (digitalPinToAnalogChannel(ANALOG_INPUT_PIN) == -1) {
        Serial.printf("TaskAnalogRead cannot work because the given pin %d cannot be used for ADC - the task will delete itself.\n", ANALOG_INPUT_PIN);
        analog_read_task_handle = NULL;  // Prevent calling vTaskDelete on non-existing task
        vTaskDelete(NULL);  // Delete this task
    }

    Serial.print("TaskAnalogRead Core ID: ");
    Serial.println(xPortGetCoreID());
    
    for (;;) {
        // Read the input on the analog pin
        int sensorValue = analogRead(ANALOG_INPUT_PIN);
        // Print the potentiometer value to the Serial Monitor
        Serial.print("Potentiometer Value: ");
        Serial.println(sensorValue);

        vTaskDelay(pdMS_TO_TICKS(100));  // Sampling rate: 100ms (FreeRTOS non-blocking delay)
    }
}
