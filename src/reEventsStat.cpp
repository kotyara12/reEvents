#include "reEventsStat.h"
#include "reMqtt.h"
#include "rLog.h"
#include "rStrings.h"
#include "sys/queue.h"
#include <string.h>

typedef struct event_count_entry_t {
  event_count_t data;
  STAILQ_ENTRY(event_count_entry_t) next;
} event_count_entry_t;
typedef struct event_count_entry_t* event_count_handle_t;

STAILQ_HEAD(event_count_head_t, event_count_entry_t);
typedef struct event_count_head_t *event_count_head_handle_t;

static event_count_head_handle_t statEvents = nullptr;

static const char* tagLOG = "EVST";
static char* _mqttTopicStat = nullptr;

char* mqttTopicEventStatCreate(const bool primary)
{
  if (_mqttTopicStat) free(_mqttTopicStat);
  _mqttTopicStat = mqttGetTopic1(primary, CONFIG_MQTT_EVENT_LOOP_STATISTIC_LOCAL, CONFIG_MQTT_EVENT_LOOP_STATISTIC_TOPIC);
  rlog_i(tagLOG, "Generated topic for publishing event statistic: [ %s ]", _mqttTopicStat);
  return _mqttTopicStat;
}

char* mqttTopicEventStatGet()
{
  return _mqttTopicStat;
}

void mqttTopicEventStatFree()
{
  if (_mqttTopicStat) free(_mqttTopicStat);
  _mqttTopicStat = nullptr;
  rlog_d(tagLOG, "Topic for publishing event statistic has been scrapped");
}

bool eventStatInit()
{
  if (!statEvents) {
    statEvents = new event_count_head_t;
    if (statEvents) {
      STAILQ_INIT(statEvents);
      return true;
    } else {
      rlog_e(tagLOG, "Statistics list initialization error!");
      return false;
    }
  };
  return true;
}

void eventStatFree()
{
  if (statEvents) {
    event_count_handle_t item, tmp;
    STAILQ_FOREACH_SAFE(item, statEvents, next, tmp) {
      STAILQ_REMOVE(statEvents, item, event_count_entry_t, next);
      delete item;
    };
    mqttTopicEventStatFree();
    delete statEvents;
  };
}

void eventStatFix(esp_event_base_t event_base, int32_t event_id)
{
  if (statEvents) {
    event_count_handle_t item = nullptr;

    // Search an existing list
    STAILQ_FOREACH(item, statEvents, next) {
      if ((strcasecmp(item->data.event_base, event_base) == 0) && (item->data.event_id == event_id)) {
        item->data.count++;
        item->data.last = time(nullptr);
        return;
      };
    };
    // Create new item
    item = new event_count_entry_t;
    item->data.event_base = event_base;
    item->data.event_id = event_id;
    item->data.count = 1;
    item->data.last = time(nullptr);
    STAILQ_INSERT_TAIL(statEvents, item, next);
  };
}

// -----------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------- MQTT --------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------

void eventStatMqttPublish()
{
  if ((_mqttTopicStat) && mqttIsConnected()) {
    uint32_t i = 0;
    char* json = nullptr;
    char* part = nullptr;
    char* temp = nullptr;
    char strtime[20];
    struct tm timeinfo;
    event_count_handle_t item = nullptr;

    STAILQ_FOREACH(item, statEvents, next) {
      i++;
      localtime_r(&item->data.last, &timeinfo);
      strftime(strtime, sizeof(strtime), CONFIG_FORMAT_DTS, &timeinfo);
      part = malloc_stringf("\"event_%.2d\":{\"base\":\"%s\",\"id\":%d,\"count\":%d,\"last\":\"%s\"}",
        i, item->data.event_base, item->data.event_id, item->data.count, strtime);
      if (part) {
        if (json) {
          temp = malloc_stringf("%s,%s", json, part);
          if (temp) {
            free(json);
            json = temp;
          };
        } else {
          json = malloc_string(part);
        };
        free(part);
      };
    };

    if (json) {
      mqttPublish(_mqttTopicStat, malloc_stringf("{%s}", json), 
        CONFIG_MQTT_EVENT_LOOP_STATISTIC_QOS, CONFIG_MQTT_EVENT_LOOP_STATISTIC_RETAINED, false, false, true);
      free(json);
    };
  };
}

// -----------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------- Events handlers ---------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------

static void eventStatEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
  eventStatFix(event_base, event_id);
}

static void eventMqttEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
  // MQTT connected
  if (event_id == RE_MQTT_CONNECTED) {
    re_mqtt_event_data_t* data = (re_mqtt_event_data_t*)event_data;
    mqttTopicEventStatCreate(data->primary);
  } 
  // MQTT disconnected
  else if ((event_id == RE_MQTT_CONN_LOST) || (event_id == RE_MQTT_CONN_FAILED)) {
    mqttTopicEventStatFree();
  };
}

bool eventStatStart()
{
  if (!statEvents) {
    eventStatInit();
  };

  if (statEvents) {
    return eventHandlerRegister(ESP_EVENT_ANY_BASE, ESP_EVENT_ANY_ID, &eventStatEventHandler, nullptr)
        && eventHandlerRegister(RE_MQTT_EVENTS, ESP_EVENT_ANY_ID, &eventMqttEventHandler, nullptr);
  };

  return false;
}