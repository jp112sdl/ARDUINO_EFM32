%1/bin/openocd.exe -f interface/jlink.cfg -f target/efm32.cfg -c "program %2 verify reset exit"