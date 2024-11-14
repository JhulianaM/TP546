//////////////////////////////////////////////////////////////////////////////
#define BLYNK_TEMPLATE_ID "TMPL2oB2u6xzG"
#define BLYNK_TEMPLATE_NAME "Pulse Oxymeter"
#define BLYNK_AUTH_TOKEN "piBOr6Q-jTuJ_Fx-LpZwGv1UbZ18hIpL"
#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#define REPORTING_PERIOD_MS     1000
// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "WLL-Inatel";
char pass[] = "inatelsemfio";
char auth[] = BLYNK_AUTH_TOKEN;
//////////////////////////////////////////////////
// PulseOximeter 
//  * beat detection reporting
//  * heart rate calculation
//  * SpO2 (oxidation level) calculation

// Definir rangos normales
#define MIN_HEART_RATE 60   // Frecuencia cardíaca mínima normal (BPM)
#define MAX_HEART_RATE 100  // Frecuencia cardíaca máxima normal (BPM)
#define MIN_SPO2 95         // Saturación de oxígeno mínima normal (%)

PulseOximeter pox;
float BPM, SpO2;
uint32_t tsLastReport = 0;

// Callback (registered below) fired when a pulse is detected
void onBeatDetected()
{
    Serial.println("Beat!");
}

void setup()
{
    Serial.begin(115200);
    Blynk.begin(auth, ssid, pass);

    Serial.print("Initializing pulse oximeter..");

    // Initialize the PulseOximeter instance
    // Failures are generally due to an improper I2C wiring, missing power supply
    // or wrong target chip
    if (!pox.begin()) {
        Serial.println("FAILED");
        for(;;);
    } else {
        Serial.println("SUCCESS");
    }

    pox.setOnBeatDetectedCallback(onBeatDetected);
}

void loop()
{
    //call update 
    pox.update();
    Blynk.run();

    BPM = pox.getHeartRate();
    SpO2 = pox.getSpO2();

    // Asynchronously dump heart rate and oxidation levels to the serial
    if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
        Serial.print("Heart rate:");
        Serial.print(pox.getHeartRate());
        Serial.print("bpm / SpO2:");
        Serial.print(pox.getSpO2());
        Serial.println("%");

        Blynk.virtualWrite(V4, BPM);
        Blynk.virtualWrite(V5, SpO2);

        // Verificar si a frecuencia cardíaca está fora de rango
        if (BPM > 0) {  // Asegurar que la lectura es válida
            if (BPM < MIN_HEART_RATE) {
                Serial.println("Aviso: Frequência cardíaca baixa!");
                // Enviar notificación a través de Blynk
                Blynk.logEvent("low_heart_rate", "Baixa frequência cardíaca");
            } else if (BPM > MAX_HEART_RATE) {
                Serial.println("Aviso: Frequência cardíaca elevada!");
                Blynk.logEvent("high_heart_rate", "Frequência cardíaca elevada");
            }
        }

        // Verificar si SpO₂ está fora de rango
        if (SpO2 > 0) {  // Asegurar que la lectura é válida
            if (SpO2 < MIN_SPO2) {
                Serial.println("¡Aviso: Saturação de oxigênio baixa!");
                Blynk.logEvent("low_spo2", "Baixa saturação de oxigênio");
            }
        }

        tsLastReport = millis();
    }
}
