# Makefile Hacking

主要想知道MiCO的SDK Demo的Makefile是怎么组织、编译整个源代码的。

## Makefile中添加调试信息方法

* 往Makefile中添加调试信息方法，方便跟踪Makefile信息：
    ```Shell 
    $(info [message])
    $(info $(variable name))
    $(info [message] $(variable name) ...)
    $(info $(variable name) [message] ...)
    ```

## makefiles/micoder\_host\_cmd.mk

* 该文件被Makefile调用，会获取当前系统的相关信息，譬如是Windows系统，还是Linux系统等等信息；
    ```Shell
    #define BUILD_STRING, MiCO toolchain commands on different hosts
    include $(MAKEFILES_PATH)/micoder_host_cmd.mk
    ```
* 默认的JTAG的配置：
    ```Shell
    JTAG         ?= jlink_swd
    ```
* 打开Makefile执行命令信息 
    ```Shell
    VERBOSE=1
    ```

## Makefile解析

* 在执行Makefile文件，命令参数保存在变量MAKECMDGOALS中，例如执行`bluetooth.ble_hello_sensor@MK3239 total`：
    ```Shell
    MAKECMDGOALS = bluetooth.ble_hello_sensor@MK3239 total
    ```
* 先clean项目，之后运行`bluetooth.ble_hello_sensor@MK3239 total`，会重新运行：
    ```Shell
    ifneq ($(BUILD_STRING),)
    -include build/$(CLEANED_BUILD_STRING)/config.mk
    # Now we know the target architecture - include all toolchain makefiles and check one of them can handle the architecture
    include $(MAKEFILES_PATH)/micoder_toolchain_GCC.mk

    build/$(CLEANED_BUILD_STRING)/config.mk: $(SOURCE_ROOT)Makefile $(MAKEFILES_PATH)/mico_target_config.mk $(MAKEFILES_PATH)/micoder_host_cmd.mk $(MAKEFILES_PATH)/micoder_toolchain_GCC.mk $(MiCO_SDK_MAKEFILES)
        $(QUIET)$(ECHO) $(if $(MiCO_SDK_MAKEFILES),Applying changes made to: $?,Making config file for first time)
        $(QUIET)$(MAKE) -r $(SILENT) -f $(MAKEFILES_PATH)/mico_target_config.mk $(CLEANED_BUILD_STRING)
    endif
    ```
  * `MAKE    := "$(COMMON_TOOLS_PATH)make$(EXECUTABLE_SUFFIX)"`
  * `EXECUTABLE_SUFFIX  := .exe`
  * 输出信息：
    ```Shell
    echo Making config file for first time
    Making config file for first time
    "./MiCoder/cmd/Win32/make.exe" -r  -f .//makefiles/mico_target_config.mk bluetooth.ble_hello_sensor@MK3239
    ```
  * `build/$(CLEANED_BUILD_STRING)/config.mk`生成目标系统的`config.mk`文件
  * `make`命令参数`mico_target_config.mk`文件解析
    * `CONFIG_FILE := $(CONFIG_FILE_DIR)/config.mk`；
    * 写入config.mk文件代码：
        ```Shell
        $(CONFIG_FILE): $(MiCO_SDK_MAKEFILES) | $(CONFIG_FILE_DIR)
            $(QUIET)$(call WRITE_FILE_CREATE, $(CONFIG_FILE) ,MiCO_SDK_MAKEFILES           		+= $(MiCO_SDK_MAKEFILES))
            $(QUIET)$(call WRITE_FILE_APPEND, $(CONFIG_FILE) ,TOOLCHAIN_NAME            		:= $(TOOLCHAIN_NAME))
            $(QUIET)$(call WRITE_FILE_APPEND, $(CONFIG_FILE) ,MiCO_SDK_LDFLAGS             		+= $(strip $(MiCO_SDK_LDFLAGS)))
            $(QUIET)$(call WRITE_FILE_APPEND, $(CONFIG_FILE) ,RESOURCE_CFLAGS					+= $(strip $(MiCO_SDK_CFLAGS)))
            ...
         ``` 
    * 输出结果：
        ```Shell
        MiCO_SDK_MAKEFILES           		+= ./platform/MCU/STM32F4xx/peripherals/libraries/libraries.mk ./platform/MCU/STM32F4xx/peripherals/peripherals.mk ./platform/GCC/GCC.mk ./libraries/utilities/json_c/json_c.mk ././MiCO/security/Sodium/Sodium.mk ././MiCO/security/SRP_6a/SRP_6a.mk ./MiCO/system/qc_test/qc_test.mk ./libraries/drivers/sensor/infrared_adc/infrared_adc.mk ./libraries/drivers/sensor/light_adc/light_adc.mk ./libraries/drivers/sensor/APDS9930/APDS9930.mk ./libraries/drivers/sensor/DHT11/DHT11.mk ./libraries/drivers/sensor/BME280/BME280.mk ./libraries/drivers/rgb_led/P9813/P9813.mk ./libraries/drivers/motor/dc_motor/dc_motor.mk ./libraries/drivers/display/VGM128064/VGM128064.mk ././libraries/bluetooth/firmware/firmware.mk ././platform/MCU/STM32F4xx/STM32F4xx.mk ./libraries/utilities/utilities.mk ././MiCO/security/security.mk ././MiCO/system/system.mk ././MiCO/core/core.mk ././MiCO/net/LwIP/mico/mico.mk ././MiCO/RTOS/FreeRTOS/mico/mico.mk ./libraries/drivers/MiCOKit_EXT/MiCOKit_EXT.mk ./libraries/drivers/keypad/gpio_button/gpio_button.mk ./libraries/drivers/spi_flash/spi_flash.mk ./libraries/bluetooth/low_energy/low_energy.mk ./libraries/daemons/bt_smart/bt_smart.mk ././MiCO/MiCO.mk ./MiCO/Security/TLS/wolfSSL/wolfSSL.mk ./MiCO/net/LwIP/LwIP.mk ./MiCO/RTOS/FreeRTOS/FreeRTOS.mk ./board/MK3239/MK3239.mk ./demos/bluetooth/ble_hello_sensor/ble_hello_sensor.mk
        TOOLCHAIN_NAME            		:= GCC
        MiCO_SDK_LDFLAGS             		+= -Wl,--gc-sections -Wl,--cref -mthumb -mcpu=cortex-m4 -Wl,-A,thumb -mlittle-endian -nostartfiles -Wl,--defsym,__STACKSIZE__=800 -L ./platform/MCU/STM32F4xx/GCC -Wl,--undefined=uxTopUsedPriority -L ./board/MK3239
        RESOURCE_CFLAGS					+= -mthumb -mcpu=cortex-m4 -mlittle-endian
        ...
        ```
    * 输出组件信息代码：
        ```Shell
        # Process all the components + MiCO
        COMPONENTS += MiCO
        $(info processing components: $(COMPONENTS))
        ```
    * 输出信息：
        ```Shell
        processing components: bluetooth.ble_hello_sensor MK3239 FreeRTOS LwIP wolfSSL MiCO
        ```
* `BUILD_STRING`被执行：
    ```Shell
    $(BUILD_STRING): main_app $(if $(SFLASH),sflash_image) copy_elf_for_eclipse  $(if $(SUB_BUILD),,.gdbinit .openocd_cfg)

    main_app: build/$(CLEANED_BUILD_STRING)/config.mk $(MiCO_SDK_PRE_APP_BUILDS) $(MAKEFILES_PATH)/mico_target_build.mk
        $(QUIET)$(COMMON_TOOLS_PATH)mkdir -p $(OUTPUT_DIR)/binary $(OUTPUT_DIR)/modules $(OUTPUT_DIR)/libraries $(OUTPUT_DIR)/resources
        $(QUIET)$(MAKE) -r $(JOBSNO) $(SILENT) -f $(MAKEFILES_PATH)/mico_target_build.mk $(CLEANED_BUILD_STRING) $(PASSDOWN_TARGETS)
        $(QUIET)$(ECHO) Build complete
    ```
  * `make`命令参数`mico_target_build.mk`解析：
    * 引入前面生成的`config.mk`文件：
        ```Shell
        CONFIG_FILE := build/$(CLEANED_BUILD_STRING)/config.mk

        include $(CONFIG_FILE)
        ```
    * 引入mico_standard_targets.mk文件：
        ```Shell
        ifeq (,$(SUB_BUILD))
        ifneq (,$(EXTRA_TARGET_MAKEFILES))
        $(foreach makefile_name,$(EXTRA_TARGET_MAKEFILES),$(eval include $(makefile_name)))
        endif
        endif
        ```
