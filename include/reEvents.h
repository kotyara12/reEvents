/* 
   EN: Main event loop for ESP32 (ESP-IDF)
   RU: Основной цикл событий для ESP32 (ESP-IDF)
   --------------------------
   (с) 2021 Разживин Александр | Razzhivin Alexander
   kotyara12@yandex.ru | https://kotyara12.ru | tg: @kotyara1971
*/

#ifndef __RE_EVENTS_H__
#define __RE_EVENTS_H__

#pragma GCC diagnostic ignored "-Wunused-variable"

#include <stddef.h>
#include <stdbool.h>
#include "esp_event.h"
#include "lwip/ip_addr.h"
#include "project_config.h"
#include "rTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------- Events -------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------

// System events
static const char* RE_SYSTEM_EVENTS = "REVT_SYSTEM";

typedef enum {
  RE_SYS_STARTED = 0,
  RE_SYS_OTA,
  RE_SYS_COMMAND,
  RE_SYS_ERROR,
  RE_SYS_TELEGRAM_ERROR,
  RE_SYS_OPENMON_ERROR,
  RE_SYS_NARODMON_ERROR,
  RE_SYS_THINGSPEAK_ERROR
} re_system_event_id_t;

typedef enum {
  RE_SYS_CLEAR   = 0,
  RE_SYS_SET     = 1
} re_system_event_type_t;   

typedef struct {
  re_system_event_type_t type;
  uint32_t data;
  bool forced;
} re_system_event_data_t;

typedef struct {
  esp_err_t err_code;
} re_error_event_data_t;

// Time events
static const char* RE_TIME_EVENTS = "REVT_TIME";
typedef enum {
  RE_TIME_RTC_ENABLED = 0,
  RE_TIME_SNTP_SYNC_OK,
  RE_TIME_EVERY_MINUTE,
  RE_TIME_START_OF_HOUR,
  RE_TIME_START_OF_DAY,
  RE_TIME_START_OF_WEEK,
  RE_TIME_START_OF_MONTH,
  RE_TIME_START_OF_YEAR,
  RE_TIME_TIMESPAN_ON,
  RE_TIME_TIMESPAN_OFF,
  RE_TIME_SILENT_MODE_ON,
  RE_TIME_SILENT_MODE_OFF,
  RE_TIME_ELTARIFF_CHANGED
} re_time_event_id_t;

// GPIO events
static const char* RE_GPIO_EVENTS = "REVT_GPIO";
typedef enum {
  RE_GPIO_CHANGE = 0,
  RE_GPIO_BUTTON = 1,
  RE_GPIO_LONG_BUTTON = 2
} re_gpio_event_id_t;

// Forwarded WIFI events
static const char* RE_WIFI_EVENTS = "REVT_WIFI";

typedef enum {
  RE_WIFI_STA_INIT = 0,
  RE_WIFI_STA_STARTED,
  RE_WIFI_STA_STOPPED,
  RE_WIFI_STA_DISCONNECTED,
  RE_WIFI_STA_GOT_IP,
  RE_WIFI_STA_PING_OK,
  RE_WIFI_STA_PING_FAILED
} re_wifi_event_id_t;

// Forwarded MQTT events
static const char* RE_MQTT_EVENTS = "REVT_MQTT";

typedef enum {
  RE_MQTT_ERROR = 0,
  RE_MQTT_ERROR_CLEAR,
  RE_MQTT_CONNECTED,
  RE_MQTT_CONN_LOST,
  RE_MQTT_CONN_FAILED,
  RE_MQTT_SERVER_PRIMARY,
  RE_MQTT_SERVER_RESERVED,
  RE_MQTT_SELF_STOP,
  RE_MQTT_COLD_RESTART,
  RE_MQTT_INCOMING_DATA
} re_mqtt_event_id_t;

typedef struct {
  bool primary;
  bool local;
  char host[32];
  uint32_t port;
} re_mqtt_event_data_t;

typedef struct {
  char* topic;
  uint32_t topic_len;
  char* data;
  uint32_t data_len;
} re_mqtt_incoming_data_t;

// Pinger service events
static const char* RE_PING_EVENTS = "REVT_PINGER";

typedef enum {
  RE_PING_STARTED = 0,
  RE_PING_STOPPED,
  RE_PING_INET_AVAILABLE,
  RE_PING_INET_SLOWDOWN,
  RE_PING_INET_UNAVAILABLE,
  RE_PING_HOST_AVAILABLE,
  RE_PING_HOST_UNAVAILABLE,
  RE_PING_TG_API_AVAILABLE,
  RE_PING_TG_API_UNAVAILABLE,
  RE_PING_MQTT1_AVAILABLE,
  RE_PING_MQTT1_UNAVAILABLE,
  RE_PING_MQTT2_AVAILABLE,
  RE_PING_MQTT2_UNAVAILABLE
} re_ping_event_id_t;

typedef enum {
  PING_OK = 0,
  PING_SLOWDOWN,
  PING_UNAVAILABLE,
  PING_FAILED
} ping_state_t;

typedef struct {
    const char* host_name;
    ip_addr_t host_addr;
    uint32_t transmitted;
    uint32_t received;
    uint32_t total_time_ms;
    uint16_t duration_ms;
    float loss;
    uint8_t ttl;
    ping_state_t state;
    time_t time_unavailable;
} ping_host_data_t;

typedef struct {
  uint8_t hosts_count;
  uint8_t hosts_available;
  ping_state_t state;
  uint16_t duration_ms_min;
  uint16_t duration_ms_max;
  uint16_t duration_ms_total;
  float loss_min;
  float loss_max;
  float loss_total;
  time_t time_unavailable;
  uint32_t count_unavailable;
} ping_inet_data_t;

typedef struct {
  ping_inet_data_t inet;
  #ifdef CONFIG_PINGER_HOST_1
  ping_host_data_t host1;
  #endif // CONFIG_PINGER_HOST_1
  #ifdef CONFIG_PINGER_HOST_2
  ping_host_data_t host2;
  #endif // CONFIG_PINGER_HOST_2
  #ifdef CONFIG_PINGER_HOST_3
  ping_host_data_t host3;
  #endif // CONFIG_PINGER_HOST_3
} ping_publish_data_t;  

static const char* RE_PARAMS_EVENTS = "REVT_PARAMS";

typedef enum {
  RE_PARAMS_RESTORED = 0,
  RE_PARAMS_INTERNAL,
  RE_PARAMS_CHANGED,
  RE_PARAMS_EQUALS
} re_params_event_id_t;

// Sensors
static const char* RE_SENSOR_EVENTS = "REVT_SENSORS";

typedef enum {
  RE_SENSOR_STATUS_CHANGED       = 0
} sensor_event_id_t;

typedef struct {
  void* sensor;
  uint8_t sensor_id;
  uint8_t old_status; 
  uint8_t new_status;
} sensor_event_status_t;

// -----------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------- Main event loop --------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------

bool eventLoopCreate();
void eventLoopDelete();

// -----------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------- Registering an event handler -------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------

bool eventHandlerRegister(esp_event_base_t event_base, int32_t event_id, esp_event_handler_t event_handler, void* event_handler_arg);
bool eventHandlerUnregister(esp_event_base_t event_base, int32_t event_id, esp_event_handler_t event_handler);

// -----------------------------------------------------------------------------------------------------------------------
// ----------------------------------------------- Sending an event to a loop --------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------

bool eventLoopPost(esp_event_base_t event_base, int32_t event_id, void* event_data, size_t event_data_size, TickType_t ticks_to_wait);
bool eventLoopPostFromISR(esp_event_base_t event_base, int32_t event_id, void* event_data, size_t event_data_size, BaseType_t* task_unblocked);
bool eventLoopPostSystem(int32_t event_id, re_system_event_type_t event_type, bool event_forced = false, uint32_t event_data = 0);
bool eventLoopPostError(int32_t event_id, esp_err_t err_code);

#ifdef __cplusplus
}
#endif

#endif // __RE_EVENTS_H__
