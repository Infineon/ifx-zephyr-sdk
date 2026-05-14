# Infineon Zephyr SDK

## Set up the Infineon downstream environment:
```
example-zephyr-environment/
├── .venv
├── downstream/
│   ├── .west/
│   └── ifx-zephyr
│   └── ifx-zephyr-sdk
│   └── modules
├── external/
│   └── .west/
│   └── bootloader/
│   └── modules/
│   └── tools/
│   └── zephyr/
└── internal/
    └── .west/
    └── bootloader/
    └── modules/
    └── tools/
    └── zephyr/
```

1. clone ifx-zephyr-sdk in a new zephyr workspace and cd into the root of the repo
2. run the following commands:
```
west init -l .
west update
west zephyr-export (not sure if this is necessary)
```
3. If you need to update your OpenOCD path (remember to rebuild after making this change):
```
west config build.cmake-args -- -DOPENOCD=path/to/infineon/openocd/bin/openocd.exe
```
4. cd into the ifx-zephyr folder that was retrieved by west update (it should be a sibling folder of ifx-zephyr-sdk)
5. build and run the lvgl basic sample:
```
west build -p -b kit_pse84_eval/pse846gps2dbzc4a/m55 samples\subsys\display\lvgl --sysbuild
west flash
```
