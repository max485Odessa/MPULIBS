rem toUTF8.exe wasm_devlist_a.cpp
rem clang++ -fpack-struct=4 --sysroot /usr/local/opt/wasi-libc -I c:\projects\component\ -I c:\projects\RADIO\RADIO_SENSORS\COMMONS\ -O0 -Wall --target=wasm32 -emit-llvm -c -S wasm_devlist_a.cpp
rem clang++ -fpack-struct=4 -I c:\projects\component\ -I c:\projects\RADIO\RADIO_SENSORS\COMMONS\ -O0 -Wall --target=wasm32 -emit-llvm -c -S c:\projects\RADIO\RADIO_SENSORS\HTTPREST\WASM\DEVLIST_A\WEBPAGECORE.cpp
rem clang++ -fpack-struct=4 -I c:\projects\component\ -I c:\projects\RADIO\RADIO_SENSORS\COMMONS\ -O0 -Wall --target=wasm32 -emit-llvm -c -S c:\projects\component\wasmcanva.cpp
rem clang++ -fpack-struct=4 -I c:\projects\component\ -I c:\projects\RADIO\RADIO_SENSORS\COMMONS\ -O0 -Wall --target=wasm32 -emit-llvm -c -S c:\projects\component\wasmlib.cpp
rem clang++ -fpack-struct=4 -I c:\projects\component\ -I c:\projects\RADIO\RADIO_SENSORS\COMMONS\ -O0 -Wall --target=wasm32 -emit-llvm -c -S c:\projects\component\wasm_cbf.cpp
rem clang++ -fpack-struct=4 -I c:\projects\component\ -I c:\projects\RADIO\RADIO_SENSORS\COMMONS\ -O0 -Wall --target=wasm32 -emit-llvm -c -S c:\projects\component\wasmbaseobj.cpp
rem clang++ -fpack-struct=4 -I c:\projects\component\ -I c:\projects\RADIO\RADIO_SENSORS\COMMONS\ -O0 -Wall --target=wasm32 -emit-llvm -c -S c:\projects\component\STMSTRING.cpp
rem clang++ -fpack-struct=4 -I c:\projects\component\ -I c:\projects\RADIO\RADIO_SENSORS\COMMONS\ -O0 -Wall --target=wasm32 -emit-llvm -c -S c:\projects\RADIO\RADIO_SENSORS\COMMONS\SENSDATABASE.cpp
rem clang++ -fpack-struct=4 -I c:\projects\component\ -I c:\projects\RADIO\RADIO_SENSORS\COMMONS\ -O0 -Wall --target=wasm32 -emit-llvm -c -S c:\projects\RADIO\RADIO_SENSORS\COMMONS\iotsensorstype.cpp



clang++ -fpack-struct=4  -I c:\projects\component\  -O3 -Wall --target=wasm32 -emit-llvm -c -S .\BITMAPS\font_bernard_22_eng.cpp
clang++ -fpack-struct=4  -I c:\projects\component\  -O3 -Wall --target=wasm32 -emit-llvm -c -S .\BITMAPS\arial16_5.cpp
clang++ -fpack-struct=4  -I c:\projects\component\  -O3 -Wall --target=wasm32 -emit-llvm -c -S .\BITMAPS\arial9.cpp
clang++ -fpack-struct=4  -I c:\projects\component\  -O3 -Wall --target=wasm32 -emit-llvm -c -S .\BITMAPS\none.cpp
clang++ -fpack-struct=4  -I c:\projects\component\  -O3 -Wall --target=wasm32 -emit-llvm -c -S .\BITMAPS\temp.cpp
clang++ -fpack-struct=4  -I c:\projects\component\  -O3 -Wall --target=wasm32 -emit-llvm -c -S .\BITMAPS\temp_water.cpp
clang++ -fpack-struct=4  -I c:\projects\component\  -O3 -Wall --target=wasm32 -emit-llvm -c -S .\BITMAPS\temp_earth.cpp
clang++ -fpack-struct=4  -I c:\projects\component\  -O3 -Wall --target=wasm32 -emit-llvm -c -S .\BITMAPS\sun.cpp
clang++ -fpack-struct=4  -I c:\projects\component\  -O3 -Wall --target=wasm32 -emit-llvm -c -S .\BITMAPS\humidity.cpp

llc.exe wasm_devlist_a.ll
llc.exe wasmcanva.ll
llc.exe wasmlib.ll
llc.exe -march=wasm32 -filetype=obj wasm_cbf.ll
llc.exe -march=wasm32 -filetype=obj wasmbaseobj.ll
llc.exe -march=wasm32 -filetype=obj STMSTRING.ll
llc.exe -march=wasm32 -filetype=obj font_bernard_22_eng.ll
llc.exe arial16_5.ll
llc.exe arial9.ll
llc.exe none.ll
llc.exe temp.ll
lld-link.exe -march=wasm32 -filetype=obj temp_water.ll
llc.exe -march=wasm32 -filetype=obj temp_earth.ll
llc.exe -march=wasm32 -filetype=obj sun.ll
llc.exe -march=wasm32 -filetype=obj humidity.ll
llc.exe -march=wasm32 -filetype=obj iotsensorstype.ll
llc.exe -march=wasm32 -filetype=obj SENSDATABASE.ll
llc.exe -march=wasm32 -filetype=obj WEBPAGECORE.ll

wasm-ld --no-entry temp_water.ll font_bernard_22_eng.ll -o senslist.wasm


wasm-ld --no-entry SENSDATABASE.o wasmlib.o humidity.o none.o temp.o WEBPAGECORE.o temp_water.o temp_earth.o sun.o STMSTRING.o wasm_devlist_a.o wasmcanva.o wasm_cbf.o iotsensorstype.o wasmbaseobj.o font_bernard_22_eng.o arial16_5.o arial9.o  -o senslist.wasm
pause

rem --export-all --no-entry 
