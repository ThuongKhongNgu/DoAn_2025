#include <WiFi.h>
#include <Firebase_ESP_Client.h>

const int BUTTON_PIN = 32;
const int LED_PIN = 12;
int lastButtonState = HIGH;    // INPUT_PULLUP nên HIGH là chưa nhấn
int buttonState = HIGH;
int ledStatus = 0;             // 0: tắt, 1: bật

// WiFi
const char* WIFI_SSID = "Trieu Ninh";
const char* WIFI_PASS = "12344321";

// Firebase
const char* API_KEY = "AIzaSyCicNauI0OCVjFMnEpFBqm0OjfhL8TcUNg";
const char* DATABASE_URL = "https://nckh-8e369-default-rtdb.firebaseio.com/";

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
bool signUpOK = false;

void tokenStatusCallback(token_info_t info);

void connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Đang kết nối WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nĐã kết nối WiFi!");
}

void connectFirebase() {
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Đăng ký Firebase thành công!");
    signUpOK = true;
  } else {
    String errorMessage = String(config.signer.signupError.message.c_str());
    Serial.println("Lỗi đăng ký Firebase: " + errorMessage);
  }

  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void tokenStatusCallback(token_info_t info) {
  if (info.status == token_status_error) {
    String errorMessage = String(info.error.message.c_str());
    Serial.println("Lỗi token: " + errorMessage);
  }
}

// Hàm đọc Firebase
void readFirebaseData() {
  if (Firebase.ready() && signUpOK) {
    if (Firebase.RTDB.getString(&fbdo, "Sensor/LED")) {  // Đọc kiểu String
      String firebaseLedStatus = fbdo.stringData();      // Lấy dữ liệu dạng chuỗi
      int ledValue = firebaseLedStatus.toInt();          // Chuyển chuỗi "1"/"0" thành số
      digitalWrite(LED_PIN, ledValue);
      ledStatus = ledValue; // Cập nhật lại trạng thái LED
      Serial.println(ledValue ? "LED BẬT (Từ Firebase)" : "LED TẮT (Từ Firebase)");
    } else {
      Serial.println("❌ Đọc Firebase thất bại: " + String(fbdo.errorReason().c_str()));
    }
  }
}


// Hàm đọc nút
void Button() {
  buttonState = digitalRead(BUTTON_PIN);
  if (buttonState == LOW && lastButtonState == HIGH) { // Nhấn nút
    ledStatus = !ledStatus;  // Đảo trạng thái LED
    digitalWrite(LED_PIN, ledStatus);
    Serial.println(ledStatus ? "LED BẬT (Từ nút nhấn)" : "LED TẮT (Từ nút nhấn)");

    // Gửi trạng thái mới lên Firebase dưới dạng CHUỖI "1" hoặc "0"
    if (Firebase.ready() && signUpOK) {
      String ledStatusStr = ledStatus ? "1" : "0";  // Ép thành chuỗi
      if (Firebase.RTDB.setString(&fbdo, "Sensor/LED", ledStatusStr)) {
        Serial.println("✅ Gửi trạng thái LED (chuỗi) lên Firebase thành công");
      } else {
        Serial.println("❌ Gửi lên Firebase thất bại: " + String(fbdo.errorReason().c_str()));
      }
    }
  }
}


void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  connectWiFi();
  connectFirebase();
}

void loop() {
  readFirebaseData();  // Đọc lệnh từ Firebase
  Button();            // Kiểm tra nút nhấn
  delay(50);           // Chống dội nút
}
