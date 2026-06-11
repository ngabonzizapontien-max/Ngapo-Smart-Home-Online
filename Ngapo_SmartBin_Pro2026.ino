/*
 * ╔══════════════════════════════════════════════════════════════╗
 * ║              Ngapo Home Smart - ESP8266 AP Mode             ║
 * ║                     Commande LED Local                      ║
 * ║                        Version 1.0                          ║
 * ╚══════════════════════════════════════════════════════════════╝
 * 
 * 📡 WiFi AP : Ngapo Home Smart
 * 🔑 Password: 12345678
 * 🌐 URL     : http://192.168.4.1
 * 
 * 💡 Fonctionnalités :
 *    - Allumer/Éteindre LED
 *    - Mode clignotement
 *    - Interface web responsive
 *    - Compatible mobile/tablette/PC
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// ─────────────────────────────────────────────────────────────
//  CONFIGURATION WiFi (Point d'Accès)
// ─────────────────────────────────────────────────────────────
const char* AP_SSID     = "Ngapo Home Smart";
const char* AP_PASSWORD = "12345678";     // Minimum 8 caractères
const IPAddress AP_IP(192, 168, 4, 1);    // IP fixe du serveur

// ─────────────────────────────────────────────────────────────
//  CONFIGURATION LED
// ─────────────────────────────────────────────────────────────
#define LED_PIN  2      // GPIO2 (Pin D4 sur NodeMCU)
bool ledState = false;  // État actuel de la LED (OFF au démarrage)
bool blinkMode = false; // Mode clignotement
unsigned long lastBlink = 0;
const int BLINK_INTERVAL = 500; // 500ms pour le clignotement

// ─────────────────────────────────────────────────────────────
//  SERVEUR WEB
// ─────────────────────────────────────────────────────────────
ESP8266WebServer server(80);

// ─────────────────────────────────────────────────────────────
//  PAGE HTML (Interface Utilisateur)
// ─────────────────────────────────────────────────────────────
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="fr">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, viewport-fit=cover">
    <meta name="apple-mobile-web-app-capable" content="yes">
    <meta name="theme-color" content="#1a1a2e">
    <title>Ngapo Home Smart</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            background: linear-gradient(135deg, #1a1a2e 0%, #16213e 100%);
            min-height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
            padding: 20px;
        }
        
        .container {
            max-width: 500px;
            width: 100%;
            background: rgba(255, 255, 255, 0.1);
            backdrop-filter: blur(10px);
            border-radius: 30px;
            padding: 30px 25px;
            box-shadow: 0 15px 35px rgba(0,0,0,0.3);
            border: 1px solid rgba(255,255,255,0.2);
        }
        
        h1 {
            text-align: center;
            color: #fff;
            font-size: 28px;
            margin-bottom: 10px;
            display: flex;
            align-items: center;
            justify-content: center;
            gap: 10px;
        }
        
        .subtitle {
            text-align: center;
            color: #a0a0c0;
            font-size: 14px;
            margin-bottom: 30px;
            padding-bottom: 20px;
            border-bottom: 1px solid rgba(255,255,255,0.1);
        }
        
        /* Status LED */
        .led-status {
            text-align: center;
            margin-bottom: 30px;
        }
        
        .led-icon {
            width: 100px;
            height: 100px;
            margin: 0 auto 15px;
            background: #2a2a4a;
            border-radius: 50%;
            display: flex;
            align-items: center;
            justify-content: center;
            transition: all 0.3s ease;
            border: 3px solid rgba(255,255,255,0.2);
        }
        
        .led-icon.on {
            background: radial-gradient(circle, #ff4444, #cc0000);
            box-shadow: 0 0 30px rgba(255,0,0,0.5);
            border-color: #ff6666;
        }
        
        .led-icon.off {
            background: #2a2a4a;
            box-shadow: none;
        }
        
        .led-icon svg {
            width: 50px;
            height: 50px;
        }
        
        .led-text {
            font-size: 18px;
            font-weight: bold;
            color: #fff;
        }
        
        .led-text.on { color: #ff4444; }
        .led-text.off { color: #888; }
        
        /* Boutons */
        .button-group {
            display: flex;
            gap: 15px;
            margin-bottom: 25px;
            flex-wrap: wrap;
        }
        
        .btn {
            flex: 1;
            padding: 15px 20px;
            font-size: 16px;
            font-weight: bold;
            border: none;
            border-radius: 15px;
            cursor: pointer;
            transition: all 0.3s ease;
            font-family: inherit;
            display: flex;
            align-items: center;
            justify-content: center;
            gap: 8px;
        }
        
        .btn-primary {
            background: #4CAF50;
            color: white;
            box-shadow: 0 4px 15px rgba(76,175,80,0.3);
        }
        
        .btn-primary:hover {
            background: #45a049;
            transform: translateY(-2px);
        }
        
        .btn-danger {
            background: #f44336;
            color: white;
            box-shadow: 0 4px 15px rgba(244,67,54,0.3);
        }
        
        .btn-danger:hover {
            background: #da190b;
            transform: translateY(-2px);
        }
        
        .btn-warning {
            background: #ff9800;
            color: white;
            box-shadow: 0 4px 15px rgba(255,152,0,0.3);
        }
        
        .btn-warning:hover {
            background: #e68900;
            transform: translateY(-2px);
        }
        
        .btn:active {
            transform: translateY(1px);
        }
        
        /* Info box */
        .info-box {
            background: rgba(0,0,0,0.3);
            border-radius: 15px;
            padding: 15px;
            margin-top: 20px;
        }
        
        .info-item {
            display: flex;
            justify-content: space-between;
            padding: 8px 0;
            color: #ddd;
            font-size: 14px;
            border-bottom: 1px solid rgba(255,255,255,0.05);
        }
        
        .info-item:last-child {
            border-bottom: none;
        }
        
        .info-label {
            font-weight: 500;
            color: #aaa;
        }
        
        .info-value {
            font-weight: bold;
            color: #4CAF50;
        }
        
        /* Footer */
        .footer {
            text-align: center;
            margin-top: 25px;
            padding-top: 15px;
            border-top: 1px solid rgba(255,255,255,0.1);
            font-size: 11px;
            color: #888;
        }
        
        /* Animation */
        @keyframes pulse {
            0%, 100% { opacity: 1; }
            50% { opacity: 0.5; }
        }
        
        .blinking {
            animation: pulse 0.5s ease-in-out infinite;
        }
        
        /* Responsive */
        @media (max-width: 480px) {
            .container { padding: 20px; }
            h1 { font-size: 24px; }
            .btn { padding: 12px 15px; font-size: 14px; }
            .led-icon { width: 80px; height: 80px; }
        }
        
        /* Touch feedback pour mobile */
        @media (hover: none) {
            .btn:active {
                transform: scale(0.97);
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>
            🏠 Ngapo Home Smart
        </h1>
        <div class="subtitle">
            Contrôle LED sans internet • Mode AP
        </div>
        
        <div class="led-status">
            <div class="led-icon" id="ledIcon">
                <svg viewBox="0 0 24 24" fill="currentColor">
                    <path d="M12 2C8.13 2 5 5.13 5 9c0 2.38 1.19 4.47 3 5.74V17c0 .55.45 1 1 1h6c.55 0 1-.45 1-1v-2.26c1.81-1.27 3-3.36 3-5.74 0-3.87-3.13-7-7-7zM9 21c0 .55.45 1 1 1h4c.55 0 1-.45 1-1v-1H9v1z"/>
                </svg>
            </div>
            <div class="led-text" id="ledText">LED ÉTEINTE</div>
        </div>
        
        <div class="button-group">
            <button class="btn btn-primary" onclick="sendCommand('on')">
                💡 ALLUMER
            </button>
            <button class="btn btn-danger" onclick="sendCommand('off')">
                ⚫ ÉTEINDRE
            </button>
        </div>
        
        <div class="button-group">
            <button class="btn btn-warning" id="blinkBtn" onclick="toggleBlink()">
                🔄 MODE CLIGNOTEMENT
            </button>
        </div>
        
        <div class="info-box">
            <div class="info-item">
                <span class="info-label">📡 WiFi AP</span>
                <span class="info-value">Ngapo Home Smart</span>
            </div>
            <div class="info-item">
                <span class="info-label">🌐 IP du serveur</span>
                <span class="info-value" id="ipAddress">192.168.4.1</span>
            </div>
            <div class="info-item">
                <span class="info-label">📱 Appareils connectés</span>
                <span class="info-value" id="clientsCount">--</span>
            </div>
            <div class="info-item">
                <span class="info-label">🔧 Mode actuel</span>
                <span class="info-value" id="modeStatus">Manuel</span>
            </div>
            <div class="info-item">
                <span class="info-label">⏱️ Dernière action</span>
                <span class="info-value" id="lastAction">--:--:--</span>
            </div>
        </div>
        
        <div class="footer">
            ESP8266 • Point d'Accès Local • Aucune connexion internet requise
        </div>
    </div>
    
    <script>
        let blinkActive = false;
        
        async function sendCommand(action) {
            try {
                const response = await fetch('/control?cmd=' + action);
                const data = await response.json();
                
                updateUI(data.ledState);
                
                // Mettre à jour l'heure de dernière action
                const now = new Date();
                document.getElementById('lastAction').textContent = now.toLocaleTimeString();
                
                if (action === 'on' || action === 'off') {
                    blinkActive = false;
                    document.getElementById('blinkBtn').innerHTML = '🔄 MODE CLIGNOTEMENT';
                    document.getElementById('modeStatus').textContent = 'Manuel';
                }
            } catch(e) {
                console.error('Erreur:', e);
            }
        }
        
        async function toggleBlink() {
            blinkActive = !blinkActive;
            
            if (blinkActive) {
                document.getElementById('blinkBtn').innerHTML = '⏹️ ARRÊTER CLIGNOTEMENT';
                document.getElementById('modeStatus').textContent = 'Clignotement';
                await sendCommand('blink_start');
            } else {
                document.getElementById('blinkBtn').innerHTML = '🔄 MODE CLIGNOTEMENT';
                document.getElementById('modeStatus').textContent = 'Manuel';
                await sendCommand('blink_stop');
            }
        }
        
        async function getStatus() {
            try {
                const response = await fetch('/status');
                const data = await response.json();
                updateUI(data.ledState);
                document.getElementById('clientsCount').textContent = data.clients;
                document.getElementById('modeStatus').textContent = data.blinkMode ? 'Clignotement' : 'Manuel';
                if (data.blinkMode && !blinkActive) {
                    blinkActive = true;
                    document.getElementById('blinkBtn').innerHTML = '⏹️ ARRÊTER CLIGNOTEMENT';
                } else if (!data.blinkMode && blinkActive) {
                    blinkActive = false;
                    document.getElementById('blinkBtn').innerHTML = '🔄 MODE CLIGNOTEMENT';
                }
            } catch(e) {
                console.error('Erreur:', e);
            }
        }
        
        function updateUI(ledState) {
            const ledIcon = document.getElementById('ledIcon');
            const ledText = document.getElementById('ledText');
            
            if (ledState === 1 || ledState === true) {
                ledIcon.className = 'led-icon on';
                ledText.className = 'led-text on';
                ledText.innerHTML = '💡 LED ALLUMÉE';
            } else {
                ledIcon.className = 'led-icon off';
                ledText.className = 'led-text off';
                ledText.innerHTML = '⚫ LED ÉTEINTE';
            }
        }
        
        // Mise à jour automatique toutes les 2 secondes
        setInterval(getStatus, 2000);
        
        // Récupération initiale de l'IP
        document.getElementById('ipAddress').textContent = window.location.hostname;
        getStatus();
    </script>
</body>
</html>
)rawliteral";

// ─────────────────────────────────────────────────────────────
//  GESTION DE LA LED
// ─────────────────────────────────────────────────────────────
void turnLEDOn() {
  digitalWrite(LED_PIN, HIGH);
  ledState = true;
  Serial.println("[LED] ALLUMÉE");
}

void turnLEDOff() {
  digitalWrite(LED_PIN, LOW);
  ledState = false;
  Serial.println("[LED] ÉTEINTE");
}

void toggleLED() {
  if (ledState) {
    turnLEDOff();
  } else {
    turnLEDOn();
  }
}

// ─────────────────────────────────────────────────────────────
//  GESTION DU MODE CLIGNOTEMENT
// ─────────────────────────────────────────────────────────────
void startBlinkMode() {
  blinkMode = true;
  Serial.println("[MODE] Clignotement activé");
}

void stopBlinkMode() {
  blinkMode = false;
  turnLEDOff();
  Serial.println("[MODE] Clignotement désactivé");
}

void handleBlink() {
  if (!blinkMode) return;
  
  unsigned long now = millis();
  if (now - lastBlink >= BLINK_INTERVAL) {
    lastBlink = now;
    toggleLED();
  }
}

// ─────────────────────────────────────────────────────────────
//  ROUTES DU SERVEUR WEB
// ─────────────────────────────────────────────────────────────
void handleRoot() {
  server.send_P(200, "text/html", index_html);
}

void handleControl() {
  if (server.hasArg("cmd")) {
    String command = server.arg("cmd");
    
    if (command == "on") {
      stopBlinkMode();
      turnLEDOn();
    }
    else if (command == "off") {
      stopBlinkMode();
      turnLEDOff();
    }
    else if (command == "blink_start") {
      startBlinkMode();
    }
    else if (command == "blink_stop") {
      stopBlinkMode();
    }
    
    // Réponse JSON
    String json = "{\"ledState\":" + String(ledState ? "1" : "0") + "}";
    server.send(200, "application/json", json);
  } else {
    server.send(400, "text/plain", "Commande manquante");
  }
}

void handleStatus() {
  int clients = WiFi.softAPgetStationNum();
  String json = "{";
  json += "\"ledState\":" + String(ledState ? "1" : "0") + ",";
  json += "\"blinkMode\":" + String(blinkMode ? "true" : "false") + ",";
  json += "\"clients\":" + String(clients);
  json += "}";
  
  server.send(200, "application/json", json);
}

void handleNotFound() {
  server.send(404, "text/plain", "Page non trouvée");
}

// ─────────────────────────────────────────────────────────────
//  SETUP
// ─────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  delay(100);
  
  Serial.println();
  Serial.println("╔════════════════════════════════════════╗");
  Serial.println("║      Ngapo Home Smart - ESP8266       ║");
  Serial.println("║         Commande LED locale            ║");
  Serial.println("╚════════════════════════════════════════╝");
  
  // Configuration LED
  pinMode(LED_PIN, OUTPUT);
  turnLEDOff();
  Serial.println("[LED] Initialisée - État OFF");
  
  // Configuration Point d'Accès
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(AP_IP, AP_IP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  
  Serial.println("\n📡 POINT D'ACCÈS ACTIF");
  Serial.println("────────────────────────────────────────");
  Serial.printf("  SSID      : %s\n", AP_SSID);
  Serial.printf("  Password  : %s\n", AP_PASSWORD);
  Serial.printf("  IP        : %s\n", WiFi.softAPIP().toString().c_str());
  Serial.println("────────────────────────────────────────");
  Serial.println("\n✨ CONNEXION INSTRUCTIONS :");
  Serial.println("  1. Activez le WiFi sur votre appareil");
  Serial.println("  2. Connectez-vous au réseau : Ngapo Home Smart");
  Serial.println("  3. Ouvrez un navigateur et allez sur :");
  Serial.println("     🌐 http://192.168.4.1");
  Serial.println("────────────────────────────────────────\n");
  
  // Configuration routes HTTP
  server.on("/", handleRoot);
  server.on("/control", handleControl);
  server.on("/status", handleStatus);
  server.onNotFound(handleNotFound);
  
  server.begin();
  Serial.println("[SERVER] Serveur HTTP démarré sur le port 80");
  Serial.println("[SYSTEM] Prêt à recevoir des connexions !\n");
}

// ─────────────────────────────────────────────────────────────
//  LOOP PRINCIPAL
// ─────────────────────────────────────────────────────────────
void loop() {
  server.handleClient();  // Important : traite les requêtes HTTP
  handleBlink();          // Gère le mode clignotement
  
  // Affiche le nombre de clients connectés toutes les 10 secondes
  static unsigned long lastClientLog = 0;
  if (millis() - lastClientLog >= 10000) {
    lastClientLog = millis();
    int clients = WiFi.softAPgetStationNum();
    if (clients > 0) {
      Serial.printf("[CLIENTS] %d appareil(s) connecté(s)\n", clients);
    }
  }
}