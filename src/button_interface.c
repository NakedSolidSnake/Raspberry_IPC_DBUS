#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <button_interface.h>
#include <dbus/dbus.h>
#include <dbus_endpoint.h>


#define _1ms    1000

static void print_dbus_error(DBusError *dbus_error, char *str);
static DBusConnection *DBUS_Init(DBusError *dbus_error);
static void DBUS_BUS_Request(DBusConnection *conn, DBusError *dbus_error);
static DBusMessage *DBUS_Get_Message_Request(void);
static bool DBUS_Send_Message(DBusConnection *conn, DBusMessage *request, const char *message);

bool Button_Run(void *object, Button_Interface *button)
{
    DBusConnection *conn;
    DBusError dbus_error;
    int state = 0;
    const char *states[] = 
    {
        "ON",
        "OFF"
    };

    if(button->Init(object) == false)
        return false;

    conn = DBUS_Init(&dbus_error);
    if(!conn)
        return false;

    while(true)
    {
        while(true)
        {
            if(!button->Read(object)){
                usleep(_1ms * 100);
                break;
            }else{
                usleep( _1ms );
            }
        }

        state ^= 0x01;

        DBUS_BUS_Request(conn, &dbus_error);

        DBusMessage *request = DBUS_Get_Message_Request();
        if(request == NULL)
            return false;
        
        DBUS_Send_Message(conn, request, states[state]);

        dbus_connection_flush(conn);
        dbus_message_unref(request);        

        if (dbus_bus_release_name(conn, CLIENT_BUS_NAME, &dbus_error) == -1)
            return false;
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

static void DBUS_BUS_Request(DBusConnection *conn, DBusError *dbus_error)
{
    int ret;
    while (true)
    {
        ret = dbus_bus_request_name(conn, CLIENT_BUS_NAME, 0, dbus_error);

        if (ret == DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER)
            break;

        if (ret == DBUS_REQUEST_NAME_REPLY_IN_QUEUE)
        {
            sleep(1);
            continue;
        }
        if (dbus_error_is_set(dbus_error))
            print_dbus_error(dbus_error, "dbus_bus_get");
    }
}

static DBusMessage *DBUS_Get_Message_Request(void)
{
    return dbus_message_new_method_call(SERVER_BUS_NAME, SERVER_OBJECT_PATH_NAME,
                                                    INTERFACE_NAME, METHOD_NAME);
}

static bool DBUS_Send_Message(DBusConnection *conn, DBusMessage *request, const char *message)
{
    DBusMessageIter iter;
    dbus_message_iter_init_append(request, &iter);

    if (!dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &message))
        return false;

    if (!dbus_connection_send(conn, request, NULL))
        return false;

    return true;
}