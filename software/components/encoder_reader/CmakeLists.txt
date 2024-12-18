idf_component_register(SRCS "src/encoder_reader.c"
                       INCLUDE_DIRS "include"
                       REQUIRES mbedtls esp_timer driver)