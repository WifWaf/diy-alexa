#ifndef __SPIFFS_ASSISST_H__
#define __SPIFFS_ASSISST_H__

#include <stdio.h>
#include <stdlib.h>
#include <esp_spiffs.h>
#include <esp_log.h>

void spiffs_assist_init(esp_vfs_spiffs_conf_t *conf);
void spiffs_assist_uninit();

esp_err_t spiffs_assist_ping(const char *full_path); 
bool spiffs_assist_read(char *data, uint16_t len);
bool spiffs_assist_read_entity(void *entity, size_t size);

esp_err_t spiffs_assist_open(const char *full_path);
void spiffs_assist_close();

void spiffs_assist_get_pos(fpos_t *pos);
bool spiffs_assist_set_pos(fpos_t *pos);

#endif // __SPIFFS_ASSISST_H__