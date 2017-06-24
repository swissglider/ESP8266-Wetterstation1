



# ESP8266-Wetterstation1
> Wetterstation1

Author: [==> swissglider](https://github.com/swissglider)



### Needed libraries:
- [WifiManager by tzapu](https://github.com/tzapu/WiFiManager.git)
- [ArduinoJson by Benoit Blanchon](https://github.com/bblanchon/ArduinoJson.git)
- [Adafruit DHT Unified by Adafruit Industries](https://github.com/adafruit/Adafruit_DHT_Unified.git)
- [Grove_Barometer_HP20x by Seeed-Studio - Master](https://github.com/Seeed-Studio/Grove_Barometer_HP20x/archive/master.zip)

### Additional informations:
#### How to switch from ino to cpp
Open `.gcc-flags.json` file from the Initialized/Imported project and add `-x c++` flag at the beginning of the value of `gccDefaultCppFlags` field:
```json
{
  "execPath": "...",
  "gccDefaultCFlags": "...",
  "gccDefaultCppFlags": "-x c++ -fsyntax-only ...",
  "gccErrorLimit": 15,
  "gccIncludePaths": "...",
  "gccSuppressWarnings": false
}
```

[See PlatformIO IDE for Atom ](http://docs.platformio.org/en/latest/ide/atom.html#ide-atom-knownissues-sclarduino-manually)
