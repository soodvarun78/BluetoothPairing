# BluetoothPairing
Sample Code for Bluetooth Paring and Connect

Dependencies
GLib2, 
BlueAlsa,
GDBUS,
Bluez5

How to compile 

gcc -g -o ble_pairing.out ble_pairing.c $(pkg-config --cflags --libs glib-2.0 gio-2.0)

How to run 

For pairing
.
/ble_pairing.out pair

For connect

./ble_pairing.out connect


