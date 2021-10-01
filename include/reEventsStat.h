/* 
   EN: Event statistics collection module
   RU: Модуль сбора статистики по событиям
   --------------------------
   (с) 2021 Разживин Александр | Razzhivin Alexander
   kotyara12@yandex.ru | https://kotyara12.ru | tg: @kotyara1971
*/

#ifndef __RE_EVENTSTAT_H__
#define __RE_EVENTSTAT_H__

#include "reEvents.h"
#include <time.h>

typedef struct {
  esp_event_base_t event_base;
  int32_t event_id;
  uint32_t count;
  time_t last;
} event_count_t;

#ifdef __cplusplus
extern "C" {
#endif

bool eventStatInit();
void eventStatFree();
void eventStatFix(esp_event_base_t event_base, int32_t event_id);

bool eventStatStart();
void eventStatMqttPublish();

#ifdef __cplusplus
}
#endif

#endif // __RE_EVENTSTAT_H__
