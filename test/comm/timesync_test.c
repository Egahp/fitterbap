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

#include "../hal_test_impl.h"
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>
#include "fitterbap/comm/timesync.h"
#include "fitterbap/ec.h"


struct fbp_time_counter_s counter_;
typedef void * fbp_os_mutex_t;
fbp_os_mutex_t fbp_os_mutex_alloc() {return NULL;}
void fbp_os_mutex_free(fbp_os_mutex_t mutex) {(void) mutex;}
void fbp_os_mutex_lock(fbp_os_mutex_t mutex) {(void) mutex;}
void fbp_os_mutex_unlock(fbp_os_mutex_t mutex) {(void) mutex;}
struct fbp_time_counter_s fbp_time_counter() {return counter_;}


#define SETUP()                                         \
    (void) state;                                       \
    counter_.frequency = 1000;                          \
    counter_.value = 0;                                 \
    struct fbp_ts_s * self = fbp_ts_initialize()


#define TEARDOWN() \
    fbp_ts_finalize(self)

#define assert_time_within_1us(value_, expected_) \
    _assert_in_range( \
        cast_to_largest_integral_type(value_), \
        cast_to_largest_integral_type((expected_) - FBP_TIME_MICROSECOND), \
        cast_to_largest_integral_type((expected_) + FBP_TIME_MICROSECOND), __FILE__, __LINE__)

static void test_initialize(void ** state) {
    SETUP();
    assert_int_equal(0, fbp_time_counter().value);
    assert_int_equal(0, fbp_ts_time(self));
    assert_int_equal(1000, counter_.frequency);
    counter_.value = 60000;
    assert_int_equal(60000, fbp_time_counter().value);
    assert_time_within_1us(fbp_ts_time(self), FBP_TIME_MINUTE);
    assert_time_within_1us(fbp_ts_time(NULL), FBP_TIME_MINUTE);
    TEARDOWN();
}

static void test_single_exact_update(void ** state) {
    SETUP();
    counter_.value = 60000;
    fbp_ts_update(self, counter_.value, FBP_TIME_HOUR, FBP_TIME_HOUR, counter_.value);
    assert_time_within_1us(fbp_ts_time(NULL), FBP_TIME_HOUR);

    // Increment counter by one second, and verify
    counter_.value += counter_.frequency;
    assert_time_within_1us(fbp_ts_time(NULL), FBP_TIME_HOUR + FBP_TIME_SECOND);
    TEARDOWN();
}

static void test_single_inexact_update(void ** state) {
    SETUP();
    counter_.value = 60000;
    fbp_ts_update(self, 59990, FBP_TIME_HOUR - FBP_TIME_MILLISECOND, FBP_TIME_HOUR + FBP_TIME_MILLISECOND, 60010);
    assert_int_equal(FBP_TIME_HOUR, fbp_ts_time(NULL));
    TEARDOWN();
}

static void test_multiple_zero_noise(void ** state) {
    SETUP();
    counter_.value = 60000;
    int64_t time = FBP_TIME_HOUR;
    for (int i = 0; i < 32; ++i) {
        fbp_ts_update(self, counter_.value - 10,
                      time - FBP_TIME_MILLISECOND,
                      time + FBP_TIME_MILLISECOND,
                      counter_.value + 10);
        assert_int_equal(time, fbp_ts_time(NULL));
        counter_.value += 10 * counter_.frequency;
        time += 10 * FBP_TIME_SECOND;
    }
    TEARDOWN();
}

int main(void) {
    hal_test_initialize();
    const struct CMUnitTest tests[] = {
            cmocka_unit_test(test_initialize),
            cmocka_unit_test(test_single_exact_update),
            cmocka_unit_test(test_single_inexact_update),
            cmocka_unit_test(test_multiple_zero_noise),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
