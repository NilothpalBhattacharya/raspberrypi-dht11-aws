/*****************************************************************************
* Filename: dht11Publisher.c
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
#if RASPBERRY_PI
#include <wiringPi.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h> // for sleep function
#include <time.h>

#include "aws_iot_mqtt_client_interface.h"

extern IoT_Error_t mqtt_connect(char *pClientID);
extern IoT_Error_t mqtt_publish(const char *topic, const char *cPayload);

/********************************** Constants ************************************/
#define DATA_PIN    7    /*GPIO 4 */
#define MAX_PULSEWIDTH 255

/********************************* Global variables ******************************/
#if RASPBERRY_PI
uint8_t pinState = HIGH;
uint8_t dht11_data[5] = {0, 0, 0, 0, 0};
#endif

char *client_name  = "dht11_node1_pub";
const char *topic  = "node1/dht11";
char pub_msg[255];
char timeStr[30] = "04/29/20T20:07:57";

/********************************** Local Functions *******************************/
/********************************************************************************************
* Function: getSystemTime
* Description:
********************************************************************************************/
void getSystemTime(void)
{
    time_t t;
    struct tm *tmp;

    t = time(NULL);
    tmp = localtime(&t);
    
    if (tmp != NULL) {
       strftime(timeStr, sizeof(timeStr), "%DT%X", tmp);
    }
}

#if RASPBERRY_PI
/********************************************************************************************
* Function: initialize
* Description:
********************************************************************************************/
static void initialize(void)
{    
    /* Program the DHT11 data pin as OUTPUT */
    pinMode(DATA_PIN, OUTPUT);
    
    /* Pull down the data pin for 18 ms */
    digitalWrite(DATA_PIN, LOW);
    delay(18);
    
    /* Pull up the data pin for 40 us */
    digitalWrite(DATA_PIN, HIGH);
    delayMicroseconds(40);
    digitalWrite(DATA_PIN, LOW);
    
    /* Now program the data pin as INPUT to reeive the data */
    pinMode(DATA_PIN, INPUT);
    pinState = digitalRead(DATA_PIN);
}

/********************************************************************************************
* Function: readPulse
* Description: Read one pulse and return its width
********************************************************************************************/
static int readPulse(void)
{
    int pulseWidth = 0;
    
    while(digitalRead(DATA_PIN) == pinState){
        delayMicroseconds(1);
        pulseWidth++;
        if(pulseWidth > MAX_PULSEWIDTH)
        {
            //printf("Error: Pulse width exceeded max value (actual %d expected %d)\n", pulseWidth, MAX_PULSEWIDTH);
            return -1;
        }
    }
    
    pinState = digitalRead(DATA_PIN);
        
    //printf("Pulse width = %d\n", pulseWidth);
    return pulseWidth;
}
#endif

/********************************************************************************************
* Function: readData
* Description: Read data
********************************************************************************************/
#if RASPBERRY_PI
static int readData(void)
{
    int i = 0;
    int pulseWidth = 0;
    int bitPos = 0;
    int sum = 0;
    double C = 0;
    //double F = 0;
    double H = 0;
    
    for (i = 0; i < 83; i++) {
        pulseWidth = readPulse();
        
        if(pulseWidth <= 0) {
            /* Error occured, restart the cycle again */
            //printf("i = %d: Error occured, restarting...\n", i);
            return -1;
        }
        
        /* Ignore first 2 pulses
         * Consider the pulse as data, only if current pin state is LOW
         * (HIGH pulse indicates data. If data transmission is complete 
         * for the current pulse, then the pin will be in LOW) */
        if ((i >= 3) && (pinState == LOW)) {
            /* Save the bit in appropriate position of the data */
            dht11_data[bitPos / 8] = (dht11_data[bitPos / 8] << 1);
            
            if(pulseWidth > 28) {
                /* It is 1, so save it */
                dht11_data[bitPos/8] = dht11_data[bitPos/8] | 1;
            }
            /* For 0, we don't have to anything, since left shift 
             * would have created a 0 in the current bit already */
            bitPos++;
        }
    }
    
    /*printf("\nLast Bit Position = %d", bitPos);
    printf("\nData: 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x", dht11_data[0],dht11_data[1],dht11_data[2],dht11_data[3],dht11_data[4]);*/
    
    /* 5 bytes of the data would have been filled by now */
    /* Verify the checksum */
    sum = dht11_data[0] + dht11_data[1] + dht11_data[2] + dht11_data[3];
    if ((sum & 0xff) != dht11_data[4]) {
        //puts("Checksum mismatch, restarting again...");
        return -1;
    }
    
    H = (double)dht11_data[0] + ((double)dht11_data[1] / 100.0);
    C = (double)dht11_data[2] + ((double)dht11_data[3] / 100.0);
    //F = C * 9.0 / 5.0 + 32.0;
    
    //printf("Humidity = %2.2f %% Temperature = %2.2f C (%2.2f F)\n", H, C, F );
    getSystemTime();
    pub_msg[0] = '\0';
    snprintf(&pub_msg[0], 255, "{\"time\":\"%s\", \"humidity\":%2.2f, \"temperature\":%2.2f}", timeStr, H, C);
    puts(pub_msg);
    
    return 0;
}
#else
static int readData(void)
{
    double C = 32;
    double H = 50;

    getSystemTime();
    pub_msg[0] = '\0';
    snprintf(&pub_msg[0], 255, "{\"time\":\"%s\", \"humidity\":%2.2f, \"temperature\":%2.2f}", timeStr, H, C);
    puts(pub_msg);

    return 0;
}
#endif

/********************************************************************************************
* Function: main
* Description: DHT11 Publisher main loop
********************************************************************************************/
int main( void )
{
    puts("!!!! Raspberry Pi wiringPi DHT11 Temperature & Humidity monitor !!!! \n" );

#if RASPBERRY_PI
    if (wiringPiSetup() == -1 ) {
        puts("wiringPiSetup() failed");
        exit(1);
    }
#endif

    mqtt_connect(client_name);

    while (1) {
#if RASPBERRY_PI
        initialize();
#endif
        if (readData() != -1) {
            mqtt_publish(topic, pub_msg);
            sleep(5);
        }
        else {
            //Read failed so sleep for 1 sec only
            sleep(1);
        }
    }

    return(0);
}
