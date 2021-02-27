<p align="center">
  <img src="https://www.softprayog.in/images/interprocess-communication-using-dbus.png">
</p>

# DBUS
## Introdução
## Implementação
### Arquivos de Configuração
#### org.solid.button.conf
```bash
<!DOCTYPE busconfig PUBLIC
"-//freedesktop//DTD D-BUS Bus Configuration 1.0//EN"
"http://www.freedesktop.org/standards/dbus/1.0/busconfig.dtd">
<busconfig>

<policy user="root">
    <allow own="org.pi.button"/>
</policy>

<policy user="pi">
    <allow own = "org.pi.button"/>
</policy>

<policy context="default">
    <allow send_interface="org.pi.led_process"/>
    <allow send_destination="org.pi.led"/>
</policy>

</busconfig>
```
#### org.solid.led.conf
```bash
<!DOCTYPE busconfig PUBLIC
"-//freedesktop//DTD D-BUS Bus Configuration 1.0//EN"
"http://www.freedesktop.org/standards/dbus/1.0/busconfig.dtd">
<busconfig>

<policy user="root">
    <allow own="org.pi.led"/>
</policy>

<policy user="pi">
    <allow own = "org.pi.led"/>
</policy>

<policy context="default">
    <allow send_interface="org.pi.led_process"/>    
</policy>

</busconfig>
```
### launch_processes.c
```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    int pid_button, pid_led;
    int button_status, led_status;

    pid_button = fork();

    if(pid_button == 0)
    {
        //start button process
        char *args[] = {"./button_process", NULL};
        button_status = execvp(args[0], args);
        printf("Error to start button process, status = %d\n", button_status);
        abort();
    }   

    pid_led = fork();

    if(pid_led == 0)
    {
        //Start led process
        char *args[] = {"./led_process", NULL};
        led_status = execvp(args[0], args);
        printf("Error to start led process, status = %d\n", led_status);
        abort();
    }

    return EXIT_SUCCESS;
}
```
### button_process.c
```c
/*
 *
 *     add-client.c: client program, takes two numbers as input,
 *                   sends to server for addition,
 8                   gets result from server,
 *                   prints the result on the screen
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <button.h>

#include <dbus/dbus.h>

#define _1MS    1000

static void inputHandler(void);


const char *const INTERFACE_NAME = "org.pi.led_process";
const char *const SERVER_BUS_NAME = "org.pi.led";
const char *const CLIENT_BUS_NAME = "org.pi.button";
const char *const SERVER_OBJECT_PATH_NAME = "/org/pi/led_control";
const char *const CLIENT_OBJECT_PATH_NAME = "/org/pi/button";
const char *const METHOD_NAME = "led_set";

static Button_t button = {
        .gpio.pin = 7,
        .gpio.eMode = eModeInput,
        .ePullMode = ePullModePullUp,
        .eIntEdge = eIntEdgeFalling,
        // .cb = inputHandler
    };

DBusError dbus_error;
void print_dbus_error(char *str);

int main(int argc, char **argv)
{
    DBusConnection *conn;
    int ret;
    char input[80];
    static int state = 0;
    const char *states[] = 
    {
        "ON",
        "OFF"
    };

    if(Button_init(&button))
        return EXIT_FAILURE;

    dbus_error_init(&dbus_error);

    conn = dbus_bus_get(DBUS_BUS_SYSTEM, &dbus_error);

    if (dbus_error_is_set(&dbus_error))
        print_dbus_error("dbus_bus_get");

    if (!conn)
        exit(1);

    // printf("Please type two numbers: ");
    // while (fgets(input, 78, stdin) != NULL)
    while(1)
    {     
        while(1)
        {
            if(!Button_read(&button)){
                usleep(_1MS * 40);
                while(!Button_read(&button));
                usleep(_1MS * 40);
                state ^= 0x01;
                break;
            }else{
                usleep( _1MS );
            }
        }   
        

        // Get a well known name
        while (1)
        {
            ret = dbus_bus_request_name(conn, CLIENT_BUS_NAME, 0, &dbus_error);

            if (ret == DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER)
                break;

            if (ret == DBUS_REQUEST_NAME_REPLY_IN_QUEUE)
            {
                fprintf(stderr, "Waiting for the bus ... \n");
                sleep(1);
                continue;
            }
            if (dbus_error_is_set(&dbus_error))
                print_dbus_error("dbus_bus_get");
        }

        DBusMessage *request;

        if ((request = dbus_message_new_method_call(SERVER_BUS_NAME, SERVER_OBJECT_PATH_NAME,
                                                    INTERFACE_NAME, METHOD_NAME)) == NULL)
        {
            fprintf(stderr, "Error in dbus_message_new_method_call\n");
            exit(1);
        }

        DBusMessageIter iter;
        dbus_message_iter_init_append(request, &iter);
        char *ptr = states[state];
        if (!dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &ptr))
        {
            fprintf(stderr, "Error in dbus_message_iter_append_basic\n");
            exit(1);
        }

        
        if (!dbus_connection_send(conn, request, NULL))
        {
            fprintf(stderr, "Error in dbus_connection_send_with_reply\n");
            exit(1);
        }        

        dbus_connection_flush(conn);
        dbus_message_unref(request);        

        if (dbus_bus_release_name(conn, CLIENT_BUS_NAME, &dbus_error) == -1)
        {
            fprintf(stderr, "Error in dbus_bus_release_name\n");
            exit(1);
        }

        printf("Please type two numbers: ");
    }

    return 0;
}

void print_dbus_error(char *str)
{
    fprintf(stderr, "%s: %s\n", str, dbus_error.message);
    dbus_error_free(&dbus_error);
}

static void inputHandler(void)
{
    static int state = 0;
    if(!Button_read(&button)){
        usleep(_1MS * 40);
        while(!Button_read(&button));
        usleep(_1MS * 40);
        state ^= 0x01;        
        
        
    }
}
```
### led_process.c
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <ctype.h>
#include <dbus/dbus.h>
#include <led.h>

typedef void (*cb)(int led_state);

static void changeLed(int state);

const char *const INTERFACE_NAME = "org.pi.led_process";
const char *const SERVER_BUS_NAME = "org.pi.led";
const char *const OBJECT_PATH_NAME = "/org/pi/led_control";
const char *const METHOD_NAME = "led_set";

typedef struct
{
    const char *command;
    cb setLed;
    const int state;    
}Table_t;

static Table_t command[] = 
{
    {"ON" , changeLed, 1},
    {"OFF", changeLed, 0}
};

DBusError dbus_error;
void print_dbus_error(char *str);
bool isinteger(char *ptr);

LED_t led =
    {
        .gpio.pin = 0,
        .gpio.eMode = eModeOutput
    };

int main(int argc, char const *argv[])
{
    DBusConnection *conn;
    int ret;

    if (LED_init(&led))
        return EXIT_FAILURE;

    dbus_error_init(&dbus_error);

    conn = dbus_bus_get(DBUS_BUS_SYSTEM, &dbus_error);

    if (!conn)
        exit(1);

    //Get a well known name
    ret = dbus_bus_request_name(conn, SERVER_BUS_NAME, DBUS_NAME_FLAG_DO_NOT_QUEUE, &dbus_error);

    if (dbus_error_is_set(&dbus_error))
        print_dbus_error("dbus_bus_get");

    if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER)
    {
        fprintf(stderr, "Dbus: not primary owner, ret = %d\n", ret);
        exit(1);
    }

    // Handle request from clients
    while (1)
    {
        // Block for msg from client
        if (!dbus_connection_read_write_dispatch(conn, -1))
        {
            fprintf(stderr, "Not connected now.\n");
            exit(1);
        }

        DBusMessage *message;

        if ((message = dbus_connection_pop_message(conn)) == NULL)
        {
            fprintf(stderr, "Did not get message\n");
            continue;
        }

        if (dbus_message_is_method_call(message, INTERFACE_NAME, METHOD_NAME))
        {
            char *s;
            char *str1 = NULL, *str2 = NULL;
            const char space[4] = " \n\t";
            int i, j;
            bool error = false;

            if (dbus_message_get_args(message, &dbus_error, DBUS_TYPE_STRING, &s, DBUS_TYPE_INVALID))
            {
                printf("%s", s);
                // Validate received message
               
                for(int i = 0; i < (sizeof(command)/sizeof(command[0])) ; i++){
                    if(!strcmp(s, command[i].command)){
                        command[i].setLed(command[i].state);
                        break;
                    }
                }

                dbus_connection_flush(conn);

                // dbus_message_unref(dbus_error_msg);                           
            }
            else
            {
                print_dbus_error("Error getting message");
            }
        }
    }

    return 0;
}

bool isinteger(char *ptr)
{

    if (*ptr == '+' || *ptr == '-')
        ptr++;

    while (*ptr)
    {
        if (!isdigit((int)*ptr++))
            return false;
    }

    return true;
}

void print_dbus_error(char *str)
{
    fprintf(stderr, "%s: %s\n", str, dbus_error.message);
    dbus_error_free(&dbus_error);
}

static void changeLed(int state)
{
    LED_set(&led, (eState_t)state);
}
```
## Conclusão
