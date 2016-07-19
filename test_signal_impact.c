/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2016 Lukas Dresel
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <unistd.h>

#include <string.h>
#include <assert.h>

#include <pthread.h>
#include <signal.h>

void cause_sigtrap()
{
    asm("int3");
}
void cause_sigsegv()
{
    unsigned int* ptr = (void*)0x4;
    *ptr = 0xcafebabe;
}
void cause_sigalrm()
{
    alarm(1);
    bool val = false;
    while(true)
    {
        val = !val;
    }
    printf("Reached val: #%s\n", val ? "true" : "false");
    return; 
}

static void* child_run(int index)
{
    int i = 0;
    while(true)
    {
        printf("Child #%d: %x\n", index, i++);
        sleep(1);
    }
    return NULL;
}

static int test_fork_children(void (*raise_signal)())
{
    for (int i = 0; i < 4; i++)
    {
        pid_t child = fork();
        if(child == -1)
        {
            // Error
            perror("fork");
            return -1;
        }
        else if (child == 0)
        {
            // Child process
            child_run(i);
        }
    }

    // The kids are running => all cannons FIRE!
    raise_signal();
    return 0;
}
static int test_pthread_children(void (*raise_signal)())
{
    for (int i = 0; i < 4; i++)
    {
        pthread_t child_handle;
        if(0 != pthread_create(&child_handle, NULL, (void* (*)(void*))child_run, (void*)i))
        {
            // Error occured
            perror("pthread_create");
            return -1;
        }
    }
    sleep(5);
    raise_signal();
    return 0;
}

static void (*parse_signal_raiser(char* signal_descriptor))()
{
    if(strcasecmp(signal_descriptor, "SIGTRAP") == 0 || atoi(signal_descriptor) == SIGTRAP)
    {
        return cause_sigtrap;
    }
    else if(strcasecmp(signal_descriptor, "SIGSEGV") == 0 || atoi(signal_descriptor) == SIGSEGV)
    {
        return cause_sigsegv;
    }
    else if(strcasecmp(signal_descriptor, "SIGALRM") == 0 || atoi(signal_descriptor) == SIGALRM)
    {
        return cause_sigalrm;
    }

    fprintf(stderr, "Raising signal %s was not yet implemented.\n", signal_descriptor);
    assert(false);
    return NULL;
}

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        printf("Usage: %s <pthread|fork> <signal>\n", argv[0]);
        return 0;
    }

    void (*cause_signal)() = parse_signal_raiser(argv[2]);

    if(strcasecmp(argv[1], "pthread") == 0)
    {
        return test_pthread_children(cause_signal);
    }
    else if (strcasecmp(argv[1], "fork") == 0)
    {
        return test_fork_children(cause_signal);
    }
    else
    {
        fprintf(stderr, "Invalid test option %s\n", argv[1]);
        return -1;
    }
}

