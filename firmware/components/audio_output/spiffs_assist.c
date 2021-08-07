#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include "spiffs_assist.h"

#define TAG_SPA "Spiffs Assist"    

#define SPIFFSA_SEMA_TAKE do{   if(!xSemaphoreTake(x_spiffsa_sema, pdMS_TO_TICKS(500))) {       \
                                    ESP_LOGE(TAG_SPA, "Failed to take spiffsa semaphore");      \
                                    return ESP_FAIL;                                            \
                                }                                                               \
                            }   while(0)

#define SPIFFSA_SEMA_GIVE do { xSemaphoreGive(x_spiffsa_sema); } while(0)

esp_vfs_spiffs_conf_t *cfg = NULL;
SemaphoreHandle_t x_spiffsa_sema;
FILE *f;

void spiffs_assist_init(esp_vfs_spiffs_conf_t *conf)
{
    if(cfg)
        return;

    cfg = conf;
    esp_err_t ret = esp_vfs_spiffs_register(cfg);

    x_spiffsa_sema = xSemaphoreCreateBinary();
    SPIFFSA_SEMA_GIVE;

    if(ret != ESP_OK) 
    {
        if(ret == ESP_FAIL) 
            ESP_LOGE(TAG_SPA, "Failed to mount or format filesystem");
        else if (ret == ESP_ERR_NOT_FOUND) 
            ESP_LOGE(TAG_SPA, "Failed to find SPIFFS partition");
        else
            ESP_LOGE(TAG_SPA, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        return;
    }
}

void spiffs_assist_uninit()
{
    if(!cfg)
    {
        ESP_LOGE(TAG_SPA, "Cannot uninitate, esp_vfs_spiffs_conf_t is NULL");
        return;
    }
    vSemaphoreDelete(x_spiffsa_sema);
    esp_vfs_spiffs_unregister(cfg->partition_label);
}

esp_err_t spiffs_assist_ping(const char *full_path)               // check if file exists
{
    esp_err_t ret = ESP_FAIL;
    
    SPIFFSA_SEMA_TAKE;   
    f = fopen(full_path, "r");                         // open path
    if(f == NULL) 
    { 
        ESP_LOGE(TAG_SPA, "Failed to open file for reading");                         
    }
    else
    {
        struct stat st;    
        ret = (esp_err_t)stat(full_path, &st);            // 0 here represents found state (ESP_OK)
    }
    fclose(f);                                      // close file stream
    SPIFFSA_SEMA_GIVE;

    return (!ret) ? ESP_OK : ESP_FAIL;                     
}

void spiffs_assist_close()
{
    fclose(f);                   // close file stream
    SPIFFSA_SEMA_GIVE;
}

esp_err_t spiffs_assist_open(const char *full_path)
{
    SPIFFSA_SEMA_TAKE;
    f = fopen(full_path, "r");                         // open path
    
    if(f == NULL)
    { 
        ESP_LOGE(TAG_SPA, "Failed to open file for reading");
        return ESP_FAIL;                                
    } 
    return ESP_OK;         
}

bool spiffs_assist_read(char *data, uint16_t len)   // const char *file_name, 
{    
    for(uint16_t i = 0; i < len; i++)              // cycle data, up to uint16_t
    {
       int c = getc(f);                 // get char from file
       if(c == EOF)                           // end of file detected?
       {
            ESP_LOGD(TAG_SPA, "OEF Found");
            if(feof(f))                 // verify against feof
            {
                ESP_LOGI(TAG_SPA, "End of File found");
                return false;
            }
            else
                ESP_LOGE(TAG_SPA, "Unable to verify end of file (EOF != feof)");   // ?? what to do in this scenario               

            break;
       }
       data[i] = c;     // place character into buffer
    } 
    return true;
}

bool spiffs_assist_read_entity(void *entity, size_t size)
{
    fread(entity, 1, size, f);

    if(feof(f)) 
    {
        ESP_LOGI(TAG_SPA, "End of File found");
        return false;
    }
    return true;
}

void spiffs_assist_get_pos(fpos_t *pos)
{
    fgetpos(f, pos);
}

bool spiffs_assist_set_pos(fpos_t *pos)
{
    fsetpos(f, pos);

    return (feof(f)) ? false : true;
}