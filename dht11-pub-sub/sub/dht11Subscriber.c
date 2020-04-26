/*****************************************************************************
* Filename: dht11Subscriber.c
* 
* Created by BITS PILANI on 11th April 2020
* Copyright 2020 BITS PILANI WILP. All rights reserved.
* 
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
* 
* 1. Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
* 
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // for sleep function
#include "aws_iot_mqtt_client_interface.h"

extern IoT_Error_t mqtt_connect(char *pClientID);
extern IoT_Error_t mqtt_subscribe(const char *topic);
extern void mqtt_display_message(void);

/********************************* Global variables ******************************/
char *client_name = "default_sub";
const char *topic = "node1/dht11";

/********************************** Local Functions *******************************/

/********************************************************************************************
* Function: main
* Description: DHT11 Subscriber main loop
********************************************************************************************/
int main (int argc, char** argv)
{
    int ret = -1;
    int result;
    
    puts("MQTT SUB Test Code");
    ret = mqtt_connect(client_name);

    ret = mqtt_subscribe(topic);

    puts("Subcriber running in while loop");

    while(1) {
        mqtt_display_message();
    }

    return 0;
}
