idf_component_register(
  SRCS "model.c" "neural_network.cc"
  INCLUDE_DIRS "include"
  # PRIV_INCLUDE_DIRS 
  REQUIRES main
  # PRIV_REQUIRES
  )

#spiffs_create_partition_image(storage ../data FLASH_IN_PROJECT) # uncomment to upload data spiffs