################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
build-1915006655: ../c2000.syscfg
	@echo 'Building file: "$<"'
	@echo 'Invoking: SysConfig'
	"C:/ti/ccs1220/ccs/utils/sysconfig_1.15.0/sysconfig_cli.bat" -s "C:/ti/C2000Ware_5_02_00_00/.metadata/sdk.json" -d "F2837xD" --script "C:/Users/lukeh/workspace_v12/can_flash_kernel/c2000.syscfg" -o "syscfg" --compiler ccs
	@echo 'Finished building: "$<"'
	@echo ' '

syscfg/board.c: build-1915006655 ../c2000.syscfg
syscfg/board.h: build-1915006655
syscfg/board.cmd.genlibs: build-1915006655
syscfg/board.opt: build-1915006655
syscfg/board.json: build-1915006655
syscfg/pinmux.csv: build-1915006655
syscfg/c2000ware_libraries.cmd.genlibs: build-1915006655
syscfg/c2000ware_libraries.opt: build-1915006655
syscfg/c2000ware_libraries.c: build-1915006655
syscfg/c2000ware_libraries.h: build-1915006655
syscfg/clocktree.h: build-1915006655
syscfg/: build-1915006655

syscfg/%.obj: ./syscfg/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccs1220/ccs/tools/compiler/ti-cgt-c2000_22.6.0.LTS/bin/cl2000" -v28 -ml -mt --cla_support=cla1 --float_support=fpu32 --tmu_support=tmu0 --vcu_support=vcu2 -Ooff --include_path="C:/ti/C2000Ware_5_02_00_00/libraries/flash_api/f2837xd/include" --include_path="C:/Users/lukeh/workspace_v12/can_flash_kernel" --include_path="C:/Users/lukeh/workspace_v12/can_flash_kernel/device" --include_path="C:/ti/C2000Ware_5_02_00_00/driverlib/f2837xd/driverlib" --include_path="C:/ti/ccs1220/ccs/tools/compiler/ti-cgt-c2000_22.6.0.LTS/include" --advice:performance=all --define=_LAUNCHXL_F28379D --define=DEBUG --define=CPU1 --diag_suppress=10063 --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="syscfg/$(basename $(<F)).d_raw" --include_path="C:/Users/lukeh/workspace_v12/can_flash_kernel/CPU1_RAM/syscfg" --obj_directory="syscfg" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

%.obj: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccs1220/ccs/tools/compiler/ti-cgt-c2000_22.6.0.LTS/bin/cl2000" -v28 -ml -mt --cla_support=cla1 --float_support=fpu32 --tmu_support=tmu0 --vcu_support=vcu2 -Ooff --include_path="C:/ti/C2000Ware_5_02_00_00/libraries/flash_api/f2837xd/include" --include_path="C:/Users/lukeh/workspace_v12/can_flash_kernel" --include_path="C:/Users/lukeh/workspace_v12/can_flash_kernel/device" --include_path="C:/ti/C2000Ware_5_02_00_00/driverlib/f2837xd/driverlib" --include_path="C:/ti/ccs1220/ccs/tools/compiler/ti-cgt-c2000_22.6.0.LTS/include" --advice:performance=all --define=_LAUNCHXL_F28379D --define=DEBUG --define=CPU1 --diag_suppress=10063 --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" --include_path="C:/Users/lukeh/workspace_v12/can_flash_kernel/CPU1_RAM/syscfg" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


