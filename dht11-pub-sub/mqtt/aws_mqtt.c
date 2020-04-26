/*
 * @file aws_mqtt.c
 * @brief MQTT publish and subscribe functions
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>

#include "aws_iot_config.h"
#include "aws_iot_log.h"
#include "aws_iot_version.h"
#include "aws_iot_mqtt_client_interface.h"

#define HOST_ADDRESS_SIZE 255
/**
 * @brief Default cert location
 */
static char certDirectory[PATH_MAX + 1] = "../../certs";

/**
 * @brief Default MQTT HOST URL is pulled from the aws_iot_config.h
 */
static char HostAddress[HOST_ADDRESS_SIZE] = AWS_IOT_MQTT_HOST;

/**
 * @brief Default MQTT port is pulled from the aws_iot_config.h
 */
static uint32_t port = AWS_IOT_MQTT_PORT;

static void iot_subscribe_callback_handler(AWS_IoT_Client *pClient,
                                                                                   char *topicName,
                                                                                   uint16_t topicNameLen,
                                                                                  IoT_Publish_Message_Params *params,
                                                                                  void *pData)
{
    IOT_UNUSED(pData);
    IOT_UNUSED(pClient);
    //IOT_INFO("Subscribe callback");
    IOT_INFO("%.*s\t%.*s", topicNameLen, topicName, (int) params->payloadLen, (char *) params->payload);
}

static void disconnectCallbackHandler(AWS_IoT_Client *pClient,
                                      void *data) {
    IOT_WARN("MQTT Disconnect");
    IoT_Error_t rc = FAILURE;

    if (NULL == pClient) {
        return;
    }

    IOT_UNUSED(data);

    if (aws_iot_is_autoreconnect_enabled(pClient)) {
        IOT_INFO("Auto Reconnect is enabled, Reconnecting attempt will start now");
    }
    else {
        IOT_WARN("Auto Reconnect not enabled. Starting manual reconnect...");
        rc = aws_iot_mqtt_attempt_reconnect(pClient);

        if (NETWORK_RECONNECTED == rc) {
            IOT_WARN("Manual Reconnect Successful");
        }
        else {
            IOT_WARN("Manual Reconnect Failed - %d", rc);
        }
    }
}

static AWS_IoT_Client client;

IoT_Error_t mqtt_connect(char *pClientID )
{
    char rootCA[PATH_MAX + 1];
    char clientCRT[PATH_MAX + 1];
    char clientKey[PATH_MAX + 1];
    char CurrentWD[PATH_MAX + 1];
    char cPayload[100];

    int32_t i = 0;

    IoT_Error_t rc = FAILURE;

    IoT_Client_Init_Params mqttInitParams = iotClientInitParamsDefault;
    IoT_Client_Connect_Params connectParams = iotClientConnectParamsDefault;

    IOT_INFO("\nAWS IoT SDK Version %d.%d.%d-%s\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_TAG);

    getcwd(CurrentWD, sizeof(CurrentWD));
    snprintf(rootCA, PATH_MAX + 1, "%s/%s/%s", CurrentWD, certDirectory, AWS_IOT_ROOT_CA_FILENAME);
    snprintf(clientCRT, PATH_MAX + 1, "%s/%s/%s", CurrentWD, certDirectory, AWS_IOT_CERTIFICATE_FILENAME);
    snprintf(clientKey, PATH_MAX + 1, "%s/%s/%s", CurrentWD, certDirectory, AWS_IOT_PRIVATE_KEY_FILENAME);

    IOT_DEBUG("rootCA %s", rootCA);
    IOT_DEBUG("clientCRT %s", clientCRT);
    IOT_DEBUG("clientKey %s", clientKey);

    mqttInitParams.enableAutoReconnect       = false; // We enable this later below
    mqttInitParams.pHostURL                  = HostAddress;
    mqttInitParams.port                      = port;
    mqttInitParams.pRootCALocation           = rootCA;
    mqttInitParams.pDeviceCertLocation       = clientCRT;
    mqttInitParams.pDevicePrivateKeyLocation = clientKey;
    mqttInitParams.mqttCommandTimeout_ms     = 20000;
    mqttInitParams.tlsHandshakeTimeout_ms    = 5000;
    mqttInitParams.isSSLHostnameVerify       = true;
    mqttInitParams.disconnectHandler         = disconnectCallbackHandler;
    mqttInitParams.disconnectHandlerData     = NULL;

    rc = aws_iot_mqtt_init(&client, &mqttInitParams);
    if (SUCCESS != rc) {
        IOT_ERROR("aws_iot_mqtt_init returned error : %d ", rc);
        return rc;
    }

    connectParams.keepAliveIntervalInSec = 600;
    connectParams.isCleanSession         = true;
    connectParams.MQTTVersion            = MQTT_3_1_1;
    connectParams.pClientID              = pClientID;
    connectParams.clientIDLen            = (uint16_t) strlen(pClientID);
    connectParams.isWillMsgPresent       = false;

    IOT_INFO("Connecting...");
    rc = aws_iot_mqtt_connect(&client, &connectParams);

    if (SUCCESS != rc) {
        IOT_ERROR("Error(%d) connecting to %s:%d", rc, mqttInitParams.pHostURL, mqttInitParams.port);
        return rc;
    }
    /*
     * Enable Auto Reconnect functionality. Minimum and Maximum time of Exponential backoff are set in aws_iot_config.h
     *  #AWS_IOT_MQTT_MIN_RECONNECT_WAIT_INTERVAL
     *  #AWS_IOT_MQTT_MAX_RECONNECT_WAIT_INTERVAL
     */
    rc = aws_iot_mqtt_autoreconnect_set_status(&client, true);
    if (SUCCESS != rc) {
        IOT_ERROR("Unable to set Auto Reconnect to true - %d", rc);
        return rc;
    }

    return rc;
}

IoT_Error_t mqtt_subscribe(const char *topic)
{
    IoT_Error_t rc = FAILURE;
    IOT_INFO("Subscribing...");
    rc = aws_iot_mqtt_subscribe(&client, topic, 11, QOS0, iot_subscribe_callback_handler, NULL);

    if (SUCCESS != rc) {
        IOT_ERROR("Error subscribing : %d ", rc);
        return rc;
    }

    return rc;
}

void mqtt_display_message(void)
{
     // Wait for all the messages to be received
	 aws_iot_mqtt_yield(&client, 100);
}

IoT_Error_t mqtt_publish(const char *topic, const char *cPayload)
{
    IoT_Error_t rc = FAILURE;
    IoT_Publish_Message_Params paramsQOS0;

    paramsQOS0.qos = QOS0;
    paramsQOS0.payload = (void *) cPayload;
    paramsQOS0.isRetained = 0;
    paramsQOS0.payloadLen = strlen(cPayload);
    rc = aws_iot_mqtt_publish(&client, topic, 11, &paramsQOS0);

    if (SUCCESS != rc) {
        IOT_ERROR("An error occurred in the loop.\n");
    }
    else {
       IOT_INFO("Publish done\n");
    }

    return rc;
}
