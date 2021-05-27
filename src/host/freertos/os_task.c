/*
 * Copyright 2014-2021 Jetperch LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include "fitterbap/os/task.h"
#include "FreeRTOS.h"
#include "task.h"

FBP_API intptr_t fbp_os_current_task_id() {
    return ((intptr_t) xTaskGetCurrentTaskHandle());
}

void fbp_os_sleep(int64_t duration) {
    if (duration < 0) {
        return;
    }
    vTaskDelay(FBP_TIME_TO_COUNTER(duration, configTICK_RATE_HZ));
}
