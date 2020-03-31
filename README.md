# Ventilator firmware, steps & shopping list

This project is based on https://github.com/jcl5m1/ventilator. 
This project has a basic/optional LED display and adjusts to a range of ventilation breathing cycles.

### Shopping List

#### optional
- 3D printer ()

### ESP8266 Firmware
Build and deploy firmware using PlatformIO.

*note, you can can find a partially completed ESP32 project in the master branch*
### Pre steps

#### Did you install PlatformIO's CLI?

##### on mac,
1. `pip install -U https://github.com/platformio/platformio-core/archive/develop.zip`
2. `vi ~/.bashrc` (or .profile or zshrc...) and add `export PATH="$PATH:~/.platformio/penv/bin"`
3. close your terminal and reopen or `source ~/.bashrc` to reload the PATH
4. (optional) activate the virtual environment, `. activate` which should now be in the PATH
5. can you run the cli? `pio`? if not go here, https://docs.platformio.org/en/latest/installation.html

#### PlatformIO extension to VSCode
1. Just search for and install the extension `PlatformIO`
2. Open `settings.json` in VSCode (from the Command Palette type `Preferences Open Settings (JSON)`)
3. Add the following (inside `{...}`),
   ```
    "platformio-ide.customPATH": "~/.platformio/penv/bin",
    "platformio-ide.useDevelopmentPIOCore": true
   ```
    **note**, customPATH probably isn't necessary since we added it to the `PATH` 
4. Restart VSCode

## Running/Uploading the program to a board
1. use the cli an run , `pio run` this will take a minute to fetch the board firmware and build it. Look in the newly created folder */.pio* to see what is pulled down
2. once everything is built then you need to upload the program (*/src/main.cpp*) to a board, this example uploads to the ESP32 board, `pio run -e ventilator -t upload`. Note, if you have all your boards connected via a serial port then you don't need to specify `-e` or which environment to upload to. `upload_port` is pegged to a specific port, this should change as needed.

## Monitor Serial port
`pio device monitor -b 115200`

## Notes
This project doesn't need VSCode or the PlatformIO extension in VSCode. If you look closely at the commands executed, all that is needed is PlatformIO Core and the included CLI.

### Updating the toolchain

1. Update PIO, `pio upgrade --dev`

2. Find the latest PIO GCC ARM toolchain for Mac here, https://dl.bintray.com/platformio/dl-packages/
currently it is, toolchain-gccarmnoneeabi-darwin_x86_64-1.90201.191206.tar.gz, then add it to `platform_packages`

3. add a platform_packages reference in plaformio.ini
```
[esp32]
platform_packages =
    toolchain-xtensa32 @ 2.80200.200226
    framework-arduinoespressif32 @ https://github.com/espressif/arduino-esp32.git
```

4. then have the environment definition reference this
```
[env:ventilator]
board = lolin32
framework = arduino
platform = espressif32
platform_packages =
    ${esp32.platform_packages}
```

##### stuck?
- https://community.platformio.org/t/how-to-install-a-newer-toolchain-than-the-official-one/8238
- https://community.platformio.org/t/can-not-compile-arm-none-eabi-g-command-not-found/9458/14

### IDEs

##### CLion

Follow these articles
- https://docs.platformio.org/en/latest/ide/clion.html
- https://community.platformio.org/t/official-platformio-plugin-for-jetbrains-clion-ide/10653

Once CLion is setup run, `platformio project init --ide clion` to refresh and rebuild all CMake targets