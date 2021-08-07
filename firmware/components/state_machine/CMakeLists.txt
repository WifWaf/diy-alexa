idf_component_register(
  SRCS "detect_wake_word.c"
  INCLUDE_DIRS "include"
  # PRIV_INCLUDE_DIRS 
  REQUIRES main
  # PRIV_REQUIRES
  )

#spiffs_create_partition_image(storage ../data FLASH_IN_PROJECT) # uncomment to upload data spiffs