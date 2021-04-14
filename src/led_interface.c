#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <ctype.h>
#include <led_interface.h>
#include <dbus/dbus.h>
#include <dbus_endpoint.h>


static void print_dbus_error(DBusError *dbus_error, char *str);
static DBusConnection *DBUS_Init(DBusError *dbus_error);
static bool DBUS_BUS_Request(DBusConnection *conn, DBusError *dbus_error);

bool LED_Run(void *object, LED_Interface *led)
{
    DBusConnection *conn;
    DBusError dbus_error;

    if (led->Init(object) == false)
        return false;

    conn = DBUS_Init(&dbus_error);
    if(!conn)
        return false;

    if (DBUS_BUS_Request(conn, &dbus_error) == false)
        return false;

    while (true)
    {
        if (!dbus_connection_read_write_dispatch(conn, -1))
        {
           break;
        }

        DBusMessage *message;

        if ((message = dbus_connection_pop_message(conn)) == NULL)
            continue;

        if (dbus_message_is_method_call(message, INTERFACE_NAME, METHOD_NAME))
        {
            char *s;

            if (dbus_message_get_args(message, &dbus_error, DBUS_TYPE_STRING, &s, DBUS_TYPE_INVALID))
            {
                if(!strcmp(s, "ON"))
                {
                    led->Set(object, 1);
                }
                else if(!strcmp(s, "OFF"))
                {
                    led->Set(object, 0);
                }

                dbus_connection_flush(conn);                                          
            }
            else
            {
                print_dbus_error(&dbus_error, "Error getting message");
            }
            dbus_message_unref(message); 
        }
    }
    return false;
}

static void print_dbus_error(DBusError *dbus_error, char *str)
{
    fprintf(stderr, "%s: %s\n", str, dbus_error->message);
    dbus_error_free(dbus_error);
}

static DBusConnection *DBUS_Init(DBusError *dbus_error)
{
    DBusConnection *conn;

    dbus_error_init(dbus_error);

    conn = dbus_bus_get(DBUS_BUS_SYSTEM, dbus_error);

    if (dbus_error_is_set(dbus_error))
        print_dbus_error(dbus_error, "dbus_bus_get");

    return conn;
}

static bool DBUS_BUS_Request(DBusConnection *conn, DBusError *dbus_error)
{
    int ret;
    ret = dbus_bus_request_name(conn, SERVER_BUS_NAME, DBUS_NAME_FLAG_DO_NOT_QUEUE, dbus_error);    

    if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER)
        return false;

    return true;
}
