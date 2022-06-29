#!/bin/bash
tar czf package/tools_macosx-2022.6.29.tar.gz tools
tar czf package/tools_linux64-2022.6.29.tar.gz tools
tar czf package/tools_win-2022.6.29.tar.gz tools
cd hardware/arm
tar czf ../../package/ARDUINO_EFM32-2022.6.29.tar.gz EFM32
cd ../../
sha256sum package/tools_macosx-2022.6.29.tar.gz
sha256sum package/tools_linux64-2022.6.29.tar.gz
sha256sum package/tools_win-2022.6.29.tar.gz
sha256sum package/ARDUINO_EFM32-2022.6.29.tar.gz