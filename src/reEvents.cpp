#include "reEvents.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "rLog.h"

static const char* logTAG = "EVTS";

#if CONFIG_EVENT_LOOP_DEDICATED
static esp_event_loop_handle_t _eventLoop;
static const char* eventLoopTaskName = "re_events";
#endif // CONFIG_EVENT_LOOP_DEDICATED

// -----------------------------------------------------------------------------------------------------------------------
// ----------------------------------------------------- Main event loop -------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------

bool eventLoopCreate()
{
  #if CONFIG_EVENT_LOOP_DEDICATED
    // Dedicated event loop
    if (!_eventLoop) {
      // Create the event loop 
      esp_event_loop_args_t _loopCfg = {
        .queue_size = CONFIG_EVENT_LOOP_QUEUE_SIZE,
        .task_name = eventLoopTaskName,
        .task_priority = CONFIG_EVENT_LOOP_PRIORITY,
        .task_stack_size = CONFIG_EVENT_LOOP_STACK_SIZE,
        .task_core_id = CONFIG_EVENT_LOOP_CORE
      };
      esp_err_t err = esp_event_loop_create(&_loopCfg, &_eventLoop);
      if (err == ESP_OK) {
        rloga_i("Dedicated event loop created successfully");
        return true;
      } else {
        rloga_e("Failed to create event loop!");
      };
    };
  #else
    // Default event loop
    esp_err_t err = esp_event_loop_create_default();  
    if (err == ESP_OK) {
      rloga_i("Default event loop created successfully");
      return true;
    } else if (err == ESP_ERR_INVALID_STATE) {
      rloga_i("Default event loop already created");
    } else {
      rloga_e("Failed to create event loop!");
    };
  #endif // CONFIG_EVENT_LOOP_DEDICATED
  return false;
}

void eventLoopDelete()
{
  #if CONFIG_EVENT_LOOP_DEDICATED
    // Dedicated event loop
    if (_eventLoop) {
      esp_event_loop_delete(_eventLoop);
      _eventLoop = nullptr;
    };
  #endif // CONFIG_EVENT_LOOP_DEDICATED
}

// -----------------------------------------------------------------------------------------------------------------------
// --------------------------------------------- Registering an event handler --------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------

bool eventHandlerRegister(esp_event_base_t event_base, int32_t event_id, esp_event_handler_t event_handler, void* event_handler_arg)
{
  #if CONFIG_EVENT_LOOP_DEDICATED
    // Dedicated event loop
    if (_eventLoop) {
      esp_err_t err = esp_event_handler_register_with(_eventLoop, event_base, event_id, event_handler, event_handler_arg);
      if (err != ESP_OK) {
        rlog_e(logTAG, "Failed to register event handler for %s #%d", event_base, event_id);
        return false;    
      };
    } else {
      return false;
    };
  #else
    // Default event loop
    esp_err_t err = esp_event_handler_register(event_base, event_id, event_handler, event_handler_arg);
    if (err != ESP_OK) {
      rlog_e(logTAG, "Failed to register event handler for %s #%d", event_base, event_id);
      return false;    
    };
  #endif // CONFIG_EVENT_LOOP_DEDICATED
  return true;
}

bool eventHandlerUnregister(esp_event_base_t event_base, int32_t event_id, esp_event_handler_t event_handler)
{
  #if CONFIG_EVENT_LOOP_DEDICATED
    // Dedicated event loop
    if (_eventLoop) {
      esp_err_t err =  esp_event_handler_unregister_with(_eventLoop, event_base, event_id, event_handler);
      if (err != ESP_OK) {
        rlog_e(logTAG, "Failed to unregister event handler for %s #%d", event_base, event_id);
        return false;    
      };
    } else {
      return false;
    };
  #else
    // Default event loop
    esp_err_t err =  esp_event_handler_unregister(event_base, event_id, event_handler);
    if (err != ESP_OK) {
      rlog_e(logTAG, "Failed to unregister event handler for %s #%d", event_base, event_id);
      return false;    
    };
  #endif // CONFIG_EVENT_LOOP_DEDICATED
  return true;
}

// -----------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------- Sending an event to a loop ---------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------

bool eventLoopPost(esp_event_base_t event_base, int32_t event_id, void* event_data, size_t event_data_size, TickType_t ticks_to_wait)
{
  #if CONFIG_EVENT_LOOP_DEDICATED
    // Dedicated event loop
    if (_eventLoop) {
      esp_err_t err = esp_event_post_to(_eventLoop, event_base, event_id, event_data, event_data_size, ticks_to_wait);
      if (err != ESP_OK) {
        rlog_e(logTAG, "Failed to post event to \"%s\" #%d: %d (%s)", event_base, event_id, err, esp_err_to_name(err));
        return false;    
      };
    };
    return false;
  #else
    // Default event loop
    esp_err_t err = esp_event_post(event_base, event_id, event_data, event_data_size, ticks_to_wait);
    if (err != ESP_OK) {
      rlog_e(logTAG, "Failed to post event to \"%s\" #%d: %d (%s)", event_base, event_id, err, esp_err_to_name(err));
      return false;    
    };
  #endif // CONFIG_EVENT_LOOP_DEDICATED
  return true;
}

bool eventLoopPostSystem(int32_t event_id, re_system_event_type_t event_type, bool event_forced, uint32_t event_data)
{
  re_system_event_data_t data = {
    .type = event_type,
    .data = event_data,
    .forced = event_forced
  }; 
  return eventLoopPost(RE_SYSTEM_EVENTS, event_id, &data, sizeof(data), portMAX_DELAY);
}
