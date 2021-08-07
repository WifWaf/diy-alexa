      
idf_component_register(
  SRCS "kiss_fft.c" "kfc.c" "kiss_fastfir.c" "kiss_fftnd.c" "kiss_fftndr.c" "kiss_fftr.c" "fftutil.c"
  INCLUDE_DIRS "include"
  # PRIV_INCLUDE_DIRS 
  REQUIRES main
  # PRIV_REQUIRES
  )
#spiffs_create_partition_image(storage ../data FLASH_IN_PROJECT) # uncomment to upload data spiffs
