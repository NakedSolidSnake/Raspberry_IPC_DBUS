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