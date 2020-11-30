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