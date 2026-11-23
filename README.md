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

### Config-Modus starten

1. Den Taster an **GPIO4** gedrückt halten.
2. Einmal kurz **Reset** (EN) drücken.
3. Den Taster noch ca. 2 s halten, bis der ESP32 im Config-Modus ist.

Danach mit dem Headless Installer verbinden und die Werte setzen.

Nicht die BOOT-Taste (GPIO0) beim Reset halten, sonst startet der ESP32 im Download-Modus.
Ist noch keine SSID gespeichert oder steht das WLAN nach 20 s nicht, startet der Config-Modus automatisch.
