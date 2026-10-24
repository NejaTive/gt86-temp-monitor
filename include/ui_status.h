/**
 * Тексты статуса на экране (DEV и PROD — одни и те же слова).
 * Короткие, на латинице (шрифт Adafruit без кириллицы).
 */

#pragma once

#define UI_STATUS_OFFLINE "Offline"        // нет связи / не подключались
#define UI_STATUS_CONNECTING "Connecting"  // поиск BT / connect
#define UI_STATUS_STARTING "Starting"      // init ELM (ATZ / ATI / setup)
#define UI_STATUS_ONLINE "Online"          // опрос OBD идёт / SIM «подключён»
#define UI_STATUS_NO_DATA "No data"        // ELM: NO DATA
#define UI_STATUS_TIMEOUT "Timeout"        // нет ответа вовремя
