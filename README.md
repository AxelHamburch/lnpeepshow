# lnpeepshow

A bitcoinSwitch for an active webcam theater

Als Grundlage dient der fantastische bitcoinSwitch von Ben Arc. Darauf aufgebaut wird nach Aktivierung des bitcoinSwitch durch eine Lightning Zahlung eine Kette an Funktionen abgearbeitet. Das ermöglicht während eine Live Webcam Streams mit einem Servomotor einen Vorhang zu öffnen, Lichter zu schalten und eine Münze auf einem Podest rotieren zu lassen.

## Firmware flashen

Die Firmware wird mit diesem Repo über VS Code + PlatformIO auf den ESP32 gespielt (Upload).

## Parametrierung

Die Parametrierung (WLAN und Device-String) erfolgt über den ZapBox Headless Installer:
https://installer.zapbox.space/headless/

- **WLAN-SSID**
- **WLAN-Passwort**
- **Device-String** (socket / WebSocket-URL des Geräts aus der LNbits-Extension)

Die Werte werden per USB-Seriell in `/config.json` auf dem ESP32 gespeichert.

In den Config-Modus gelangt der ESP32
- automatisch, wenn noch keine SSID gespeichert ist,
- automatisch, wenn die WLAN-Verbindung nach 20 s nicht steht,
- im laufenden Betrieb durch **5 s Halten der BOOT-Taste** (GPIO0).
  Die BOOT-Taste nicht beim Einschalten/Reset halten, sonst startet der ESP32 im Download-Modus.
