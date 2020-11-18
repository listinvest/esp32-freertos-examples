/*
    currently I am learning how rtos works. I learn how to use: tasks, queues, semaphores, pinned cores

    references:
    - freertos Queue API (https://www.freertos.org/a00018.html)

*/

#include <Arduino.h>
#define ARDUINOJSON_USE_LONG_LONG 1
#include <ArduinoJson.h>                                                            

void taskOne(void *);
void taskTwo(void *);
void taskPinned(void *);
void taskSender(void *);
void taskReceiver(void *);
void taskSemaphore(void *);

QueueHandle_t queue;
int queueSize = 10;

int nTasks=5;
SemaphoreHandle_t barrierSemaphore = xSemaphoreCreateCounting(nTasks, 0);

void setup() {
    Serial.begin(115200);
    Serial.println("freeRTOS Task - Start");
    delay(1000);

    // checking core being run and priority
    Serial.print("Setup: Executing on core ");
    Serial.println(xPortGetCoreID());
    Serial.print("Setup: priority = ");
    Serial.println(uxTaskPriorityGet(NULL));

    // testing semaphores
    int semaphoreCount;

    for (semaphoreCount=0; semaphoreCount<nTasks; semaphoreCount++) {
        xTaskCreatePinnedToCore(
        taskSemaphore,              /* Task function. */
        "TaskSemaphore",            /* String with name of task. */
        10000,                      /* Stack size in bytes. */
        (void *)semaphoreCount,     /* Parameter passed as input of the task */
        0,                          /* Priority of the task. */
        NULL,                       /* Task handle. */
        0);                         /* Specified Core to Run on */
    }

    for(semaphoreCount= 0; semaphoreCount< nTasks; semaphoreCount++){
        xSemaphoreTake(barrierSemaphore, portMAX_DELAY);
    }

    Serial.println("5 Tasks launched and semaphore barrier passed...");

    // testing task create
    TaskHandle_t taskOneHandler;
    TaskHandle_t taskTwoHandler;

    xTaskCreate(
        taskOne,          /* Task function. */
        "TaskOne",        /* String with name of task. */
        10000,            /* Stack size in bytes. */
        NULL,             /* Parameter passed as input of the task */
        10,                /* Priority of the task. */
        &taskOneHandler);            /* Task handle. */

    xTaskCreate(
        taskTwo,          /* Task function. */
        "TaskTwo",        /* String with name of task. */
        10000,            /* Stack size in bytes. */
        NULL,             /* Parameter passed as input of the task */
        11,                /* Priority of the task. */
        &taskTwoHandler);            /* Task handle. */

    Serial.print("taskOne: priority = ");
    Serial.println(uxTaskPriorityGet(taskOneHandler));
    Serial.print("taskTwo: priority = ");
    Serial.println(uxTaskPriorityGet(taskTwoHandler));

    // testing pinned core
    xTaskCreatePinnedToCore(
        taskPinned,          /* Task function. */
        "TaskPinned",        /* String with name of task. */
        10000,            /* Stack size in bytes. */
        NULL,             /* Parameter passed as input of the task */
        1,                /* Priority of the task. */
        NULL,              /* Task handle. */
        0);               /* Specified Core to Run on */

    // testing queue for inter-task communications
    queue = xQueueCreate( queueSize, sizeof( int ) );
 
    if(queue == NULL){
        Serial.println("Error creating the queue");
    }

    xTaskCreate(
        taskSender,          /* Task function. */
        "TaskSender",        /* String with name of task. */
        10000,            /* Stack size in bytes. */
        NULL,             /* Parameter passed as input of the task */
        1,                /* Priority of the task. */
        NULL);            /* Task handle. */

    xTaskCreate(
        taskReceiver,          /* Task function. */
        "TaskReceiver",        /* String with name of task. */
        10000,            /* Stack size in bytes. */
        NULL,             /* Parameter passed as input of the task */
        1,                /* Priority of the task. */
        NULL);            /* Task handle. */
}

extern "C" void app_main() {
    setup();
    while(1) {
        Serial.println("Hello from app_main");
        Serial.print("main: Executing on core ");
        Serial.println(xPortGetCoreID());
        delay(3000);
    }
}

void taskOne(void * parameter) {
    Serial.print("taskOne: Executing on core ");
    Serial.println(xPortGetCoreID());
    for (int i=0; i<10; i++) {
        Serial.println("Hello from taskOne [" + String(i) + "]");
        delay(1500);
    }

    Serial.println("Ending taskOne");
    vTaskDelete( NULL );
}

void taskTwo(void * parameter) {
    Serial.print("taskTwo: Executing on core ");
    Serial.println(xPortGetCoreID());
    for (int i=0; i<10; i++) {
        Serial.println("Hello from taskTwo [" + String(i) + "]");
        delay(1300);
    }

    Serial.println("Ending taskTwo");
    vTaskDelete( NULL );
}

void taskPinned(void * parameter) {
    Serial.print("taskPinned: Executing on core ");
    Serial.println(xPortGetCoreID());
    for (int i=0; i<10; i++) {
        Serial.println("Hello from taskPinned [" + String(i) + "]");
        delay(1700);
    }

    Serial.println("Ending taskPinned");
    vTaskDelete( NULL );
}

void taskSender(void * parameter) {
    Serial.println("taskSender: Executing");
    for(int i=0; i<10; i++) {
        xQueueSend(queue, &i, portMAX_DELAY);
    }

    Serial.println("ending taskSender");
    vTaskDelete( NULL );
}

void taskReceiver(void * parameter) {
    Serial.println("taskReceiver: Executing");

    int element;
    for(int i=0; i<10; i++) {
        xQueueReceive(queue, &element, portMAX_DELAY);
        Serial.print(element);
        Serial.print("|");
    }

    Serial.println("\nending taskReceiver");
    vTaskDelete( NULL );
}

void taskSemaphore(void * parameter) {
    String taskMessage = "taskSemaphore executed number:";
    taskMessage = taskMessage + ((int)parameter);
 
    Serial.println(taskMessage);
 
    xSemaphoreGive(barrierSemaphore); 

    // Serial.println("\nending taskSemaphore");
    vTaskDelete( NULL );
}