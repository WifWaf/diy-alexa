  
idf_component_register(
        SRCS "ring_buffer.c" "i2s_sampler.c" "i2s_mic_sampler.c"
        INCLUDE_DIRS "include"
        # PRIV_INCLUDE_DIRS 
        REQUIRES log
        # PRIV_REQUIRES
        )
#spiffs_create_partition_image(storage ../data FLASH_IN_PROJECT) # uncomment to upload data spiffs