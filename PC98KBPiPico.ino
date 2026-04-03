#include <Adafruit_TinyUSB.h>
#include "98_key_map.h"

/*
 * HID Report Descriptor for a standard 6-key rollover keyboard.
 * 標準的なUSB HIDキーボードとしてのディスクリプタを定義します。
 */
uint8_t const desc_hid_report[] = {
        TUD_HID_REPORT_DESC_KEYBOARD()
};

Adafruit_USBD_HID usb_hid(desc_hid_report, sizeof(desc_hid_report), HID_ITF_PROTOCOL_KEYBOARD, 2, false);

/*
 * PC-98 Keyboard Pin Definitions (RX Only Mode)
 * TX(送信)を物理的・ソフトウェア的に完全に切り離し、受信専用とします。
 */
#define TX_PIN          0       // ~RST (負論理リセット) ピンとして使用
#define RX_PIN          1       // UART0 RX (GP1)
#define I_RDY_PIN       4       // Ready信号 (Active LOW)

/*
 * Keyboard State Management
 * modifier_state: Ctrl, Shift, Alt, GUIなどの状態をビットマスクで保持
 * key_state: 同時押しされている通常キーのUsage IDを配列で保持（最大6キー）
 */
uint8_t modifier_state = 0;
uint8_t key_state[6] = {0};

/*
 * 押下されたキーをレポート配列に追加、またはモディファイアを更新します。
 */
void pressKey(uint8_t usage_id) {
        if (usage_id >= 224 && usage_id <= 231) {
                // Modifiers (0xE0 - 0xE7)
                modifier_state |= (1 << (usage_id - 224));
        } else {
                // Standard keys
                for (int i = 0; i < 6; i++) {
                        if (key_state[i] == usage_id) return; // 既に押されている場合は無視
                }
                for (int i = 0; i < 6; i++) {
                        if (key_state[i] == 0) {
                                key_state[i] = usage_id;
                                break;
                        }
                }
        }
}

/*
 * 離上されたキーをレポート配列から削除、またはモディファイアを更新します。
 */
void releaseKey(uint8_t usage_id) {
        if (usage_id >= 224 && usage_id <= 231) {
                // Modifiers (0xE0 - 0xE7)
                modifier_state &= ~(1 << (usage_id - 224));
        } else {
                // Standard keys
                for (int i = 0; i < 6; i++) {
                        if (key_state[i] == usage_id) {
                                key_state[i] = 0;
                        }
                }
        }
}

/*
 * TinyUSB Callback
 * PC側からのLED状態を受信しますが、TXを結線しないため内部処理は行いません。
 */
void my_set_report_cb(uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {
        // Do nothing (RX Only Mode)
}

void setup() {
        // 1. USB HID機能の初期化
        usb_hid.setReportCallback(NULL, my_set_report_cb);
        usb_hid.begin();
  
        pinMode(I_RDY_PIN, OUTPUT);
        pinMode(RX_PIN, INPUT);
        
        // 負論理リセットピンを出力モードに設定
        pinMode(TX_PIN, OUTPUT);

        // 2. USBの認識が完了するまで待機
        uint32_t t = millis();
        while (!TinyUSBDevice.mounted() && (millis() - t) < 3000) {
                delay(1);
        }

        // ==========================================
        // 3. ハードウェアリセットの確実な実行
        // ==========================================
        // Reset
        digitalWrite(I_RDY_PIN, HIGH);
        digitalWrite(TX_PIN,LOW);
        delayMicroseconds(15);
        digitalWrite(TX_PIN,HIGH);
        // キーボード内のマイコンが起動するまで待機
        delay(200);

        // ==========================================
        // 4. UARTは「受信(RX)」のみ割り当てて通信開始
        // ==========================================
        Serial1.setRX(RX_PIN);
        Serial1.setTX(TX_PIN);
        Serial1.begin(19200, SERIAL_8O1);
}

void loop() {
        if (Serial1.available() > 0) {
                
                unsigned char raw_data = Serial1.read();
                
                // データ処理中およびUSB送信待機中はキーボードからの送信を保留させる
                digitalWrite(I_RDY_PIN, HIGH);
                delayMicroseconds(40);
                
                // 万が一のシステム応答はスキップ
                if (raw_data != 0xFA && raw_data != 0xFC && raw_data != 0xFB) {
                        
                        signed char data = (signed char)raw_data;
                        
                        if (data < 0) {    
                                // Break (離上)
                                data += 128;
                                releaseKey(PC98_to_UsageID[data]);
                                // ★ここでreturnせず、かつての「両対応」を再現するため下へ流す
                        } else {  
                                // Make (押下)
                                pressKey(PC98_to_UsageID[data]);
                        }

                        // if/elseブロックの外に置くことで、物理キーの
                        // 「押し込み時(Make)」と「跳ね返り時(Breakの+128補正後)」の
                        // 両方でOSへワンショットのトグル信号を送る！
                        if (data == 114 || data == 113) { 
                                uint8_t target_key = (data == 114) ? 53 : 57;
                                
                                // 1. 押下状態にしてUSBへ同期送信
                                pressKey(target_key);
                                uint32_t t = millis();
                                while (!usb_hid.ready() && (millis() - t) < 10) { delay(1); }
                                if (usb_hid.ready()) {
                                        usb_hid.keyboardReport(0, modifier_state, key_state);
                                }
                                
                                // 2. すぐに離上状態にしてUSBへ同期送信（ワンショット完了）
                                releaseKey(target_key);
                                t = millis();
                                while (!usb_hid.ready() && (millis() - t) < 10) { delay(1); }
                                if (usb_hid.ready()) {
                                        usb_hid.keyboardReport(0, modifier_state, key_state);
                                }
                        } else {
                                // Kana/Caps以外の通常キーのUSB同期送信処理
                                uint32_t t = millis();
                                while (!usb_hid.ready() && (millis() - t) < 10) { delay(1); }
                                if (usb_hid.ready()) {
                                        usb_hid.keyboardReport(0, modifier_state, key_state);
                                }
                        }
                }
                // データ処理およびUSB送信完了後、確実に受信許可(LOW)に戻す
                digitalWrite(I_RDY_PIN, LOW);
        } 
        else {
                // 受信データがない時のハンドシェイク処理
                /*
                digitalWrite(I_RDY_PIN, HIGH);
                delayMicroseconds(40);
                
                */
                digitalWrite(I_RDY_PIN, LOW);
        }
}
