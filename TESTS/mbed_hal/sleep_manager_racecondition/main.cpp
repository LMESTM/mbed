/* mbed Microcontroller Library
 * Copyright (c) 2017 ARM Limited
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

#include "utest/utest.h"
#include "unity/unity.h"
#include "greentea-client/test_env.h"

#if !DEVICE_SLEEP
#error [NOT_SUPPORTED] test not supported
#endif

using namespace utest::v1;

void sleep_manager_locking_thread_test()
{
    for (uint32_t i = 0; i < 100; i++) {
        sleep_manager_lock_deep_sleep();
        Thread::wait(25);
        sleep_manager_unlock_deep_sleep();
    }
}

void sleep_manager_multithread_test()
{
    {
        Callback<void()> cb(sleep_manager_locking_thread_test);
        Thread t1;
        Thread t2;

        t1.start(callback(cb));
        Thread::wait(25);
        t2.start(callback(cb));

        // Wait for the threads to finish
        t1.join();
        t2.join();
    }

    bool deep_sleep_allowed = sleep_manager_can_deep_sleep();
    TEST_ASSERT_TRUE_MESSAGE(deep_sleep_allowed, "Deep sleep should be allowed");
}

void sleep_manager_locking_irq_test()
{
    sleep_manager_lock_deep_sleep();
    sleep_manager_unlock_deep_sleep();
}

void sleep_manager_irq_test()
{
    {
        Ticker ticker1;
        Timer timer;

        ticker1.attach_us(&sleep_manager_locking_irq_test, 500);

        // run this for 5 seconds
        timer.start();
        int start = timer.read();
        int end = start + 5;
        while (timer.read() < end) {
            sleep_manager_locking_irq_test();
        }
        timer.stop();
    }

    bool deep_sleep_allowed = sleep_manager_can_deep_sleep();
    TEST_ASSERT_TRUE_MESSAGE(deep_sleep_allowed, "Deep sleep should be allowed");
}

utest::v1::status_t greentea_test_setup(const size_t number_of_cases) 
{
    GREENTEA_SETUP(30, "default_auto");
    return greentea_test_setup_handler(number_of_cases);
}

Case cases[] = {
    Case("sleep manager HAL - locking multithreaded", sleep_manager_multithread_test),
    Case("sleep manager HAL - locking IRQ", sleep_manager_irq_test),
};

Specification specification(greentea_test_setup, cases, greentea_test_teardown_handler);

int main() {
    Harness::run(specification);
}
