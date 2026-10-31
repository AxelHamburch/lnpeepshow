// Serial config mode, compatible with the ZapBox Web Installer (https://installer.zapbox.space)
//
// Protocol (one command per line, 115200 baud):
//   /hello                      -> logo + [CONFIG_MODE_ENTER] marker (installer detects config mode)
//   /file-read <file>           -> "/file-read <content>"
//   /file-remove <file>
//   /file-append <file> <data>
//   /file-write <file> <data>
//   /config-restart | /config-soft-reset | /config-done -> restart the ESP32
//
// The installer writes /config.json as a JSON array of {"name":..., "value":...}.
// Only the first three entries are used here: [0] ssid, [1] wifipassword, [2] socket (device string).

void configOverSerialPort() {
  executeConfig();
}

void executeConfig() {
  // The installer sends long lines (config.json ~1.3 KB) in 128 byte chunks with 15 ms gaps
  Serial.setTimeout(5000);

  Serial.flush();
  delay(100);
  while (Serial.available()) Serial.read();

  serialPrintln("");
  serialPrintln("===================================");
  serialPrintln("   Serial Config Mode Active");
  serialPrintln("===================================");
  serialPrintln("[CONFIG_MODE_ENTER]");
  serialPrintln("Waiting for commands...");

  // Repeat the marker so the installer detects it even if it connected late
  delay(200);
  serialPrintln("[CONFIG_MODE_ENTER]");
  delay(500);
  serialPrintln("[CONFIG_MODE_ENTER]");

  // Drop stale bytes only after all markers are out, so no early command gets lost
  delay(50);
  while (Serial.available()) Serial.read();

  while (true) {
    yield();
    if (Serial.available() == 0) {
      delay(10);
      continue;
    }

    String data = Serial.readStringUntil('\n');
    data.trim();
    if (data.length() == 0) continue;

    KeyValue kv = extractKeyValue(data);
    String commandName = kv.key;

    if (commandName != "/file-read") {
      if (data.length() > 100) {
        serialPrintln("received: " + commandName + " [" + String(data.length()) + " bytes]");
      } else {
        serialPrintln("received: " + data);
      }
    }

    if (commandName == "/config-done") {
      serialPrintln("/config-done");
      delay(500);
      ESP.restart();
    }
    executeCommand(commandName, kv.value);
  }
}

void executeCommand(String commandName, String commandData) {
  KeyValue kv = extractKeyValue(commandData);
  String path = kv.key;
  String data = kv.value;

  if (commandName == "/hello") {
    Serial.print("  ____  ___   ___  ___  ____  _  __\n");
    Serial.print(" /_  / / _ | / _ \\/ _ )/ __ \\| |/_/\n");
    Serial.print("  / /_/ __ |/ ___/ _  / /_/ />  <\n");
    Serial.print(" /___/_/ |_/_/  /____/\\____/_/|_|\n");
    Serial.flush();
    delay(50);
    serialPrintln("[CONFIG_MODE_ENTER]");
    return;
  }
  if (commandName == "/config-restart" || commandName == "/config-soft-reset") {
    serialPrintln("- Restarting ESP32...");
    serialPrintln("[CONFIG_MODE_EXIT]");
    delay(500);
    ESP.restart();
    return;
  }
  if (commandName == "/file-remove") {
    return removeFile(path);
  }
  if (commandName == "/file-append") {
    return appendToFile(path, data);
  }
  if (commandName == "/file-write") {
    return writeFile(path, data);
  }
  if (commandName == "/file-read") {
    return readFile(path);
  }

  serialPrintln("- Unknown command");
}

void serialPrintln(const String &msg) {
  Serial.println(msg);
  Serial.flush();
  delay(15);
}

void removeFile(String path) {
  serialPrintln("- Remove file: " + path);
  FlashFS.remove("/" + path);
}

void appendToFile(String path, String data) {
  serialPrintln("- Append to file: " + path);
  File file = FlashFS.open("/" + path, FILE_APPEND);
  if (!file) {
    file = FlashFS.open("/" + path, FILE_WRITE);
  }
  if (file) {
    file.println(data);
    file.close();
  }
  serialPrintln("- Append done");
}

void writeFile(String path, String data) {
  serialPrintln("- Write file: " + path);
  File file = FlashFS.open("/" + path, FILE_WRITE);
  if (file) {
    file.print(data);
    file.close();
    serialPrintln("- Write done");
  } else {
    serialPrintln("- Write failed");
  }
}

void readFile(String path) {
  File file = FlashFS.open("/" + path, "r");
  if (!file) {
    serialPrintln("- Failed to open file for reading");
    return;
  }
  String content = file.readString();
  file.close();

  // Response format expected by the installer: "/file-read <JSON>\n"
  String response = "/file-read " + content;
  for (unsigned int i = 0; i < response.length(); i += 64) {
    Serial.write(response.c_str() + i, min(64u, (unsigned int)response.length() - i));
    Serial.flush();
    delay(15);
  }
  Serial.println();
  Serial.flush();
  delay(50);
}

KeyValue extractKeyValue(String s) {
  int spacePos = s.indexOf(" ");
  String key = s.substring(0, spacePos);
  if (spacePos == -1) {
    return {key, ""};
  }
  String value = s.substring(spacePos + 1, s.length());
  return {key, value};
}
