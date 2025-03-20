# **Projet ESP32 : Communication LoRa & MQTT**  

##  **Description**  
Ce projet implémente un système de communication utilisant un **ESP32**, un module **LoRa** et le protocole **MQTT**. L'objectif est de recevoir des données via LoRa, les publier sur un serveur MQTT, et inversement. Ce projet est adapté aux applications IoT nécessitant une connectivité longue portée et une transmission efficace des données.

##  **Fonctionnalités**  
- **Connexion automatique au Wi-Fi** avec détection de perte de connexion et reconnexion automatique.  
- **Communication bidirectionnelle LoRa ↔️ MQTT** : messages MQTT publiés sur LoRa et vice-versa.  
- **Gestion avancée des événements réseau** (perte de Wi-Fi, reconnexion MQTT, erreurs LoRa).  
- **Souscription dynamique aux topics MQTT** pour interagir avec d'autres dispositifs IoT.  
- **Logs détaillés avec ESP_LOG** pour un suivi précis du fonctionnement du système.  
- **Utilisation de FreeRTOS** pour exécuter plusieurs tâches en parallèle de manière optimisée.  

##  **Technologies utilisées**  
- **ESP-IDF** : Environnement de développement pour ESP32.  
- **FreeRTOS** : Gestion multitâche pour optimiser les performances.  
- **LoRa** : Protocole de communication longue portée et basse consommation.  
- **MQTT (Mosquitto)** : Protocole léger basé sur le modèle `publish/subscribe`.  
- **C / C++** : Langage de programmation utilisé pour l’embarqué.  

##  **Architecture du projet**  
Le projet fonctionne selon le schéma suivant :  
1. **L’ESP32 se connecte au Wi-Fi** et vérifie la stabilité de la connexion.  
2. **Connexion au broker MQTT** et souscription au topic `kikimou`.  
3. **Initialisation du module LoRa** avec configuration de la fréquence et du CRC.  
4. **Échange de messages** :  
   - Les messages reçus sur **LoRa** sont publiés sur **MQTT** (`kikidur`).  
   - Les messages reçus sur **MQTT** (`kikimou`) sont envoyés via **LoRa**.  

##  **Installation & Configuration**  
### **1. Prérequis**  
- Un module **ESP32** avec **LoRa** intégré (ex: Heltec WiFi LoRa 32, TTGO LoRa32).  
- **ESP-IDF** installé et configuré sur votre ordinateur.  
- Accès à un **broker MQTT** (ex: Mosquitto ou [test.mosquitto.org](https://test.mosquitto.org/)).  

### **2. Compilation et Flashing**  
1. **Cloner le projet** :  
   ```bash  
   git clone https://github.com/ton-repo/esp32-lora-mqtt.git  
   cd esp32-lora-mqtt  
   ```  
2. **Configurer l'environnement ESP-IDF** :  
   ```bash  
   . $HOME/esp/esp-idf/export.sh  
   ```  
3. **Modifier les paramètres Wi-Fi et MQTT dans le code source** (`main.c`) :  
   ```c  
   #define WIFI_SSID "VOTRE_SSID"
   #define WIFI_PASS "VOTRE_MOT_DE_PASSE"
   #define MQTT_BROKER "mqtt://test.mosquitto.org"
   ```  
4. **Compiler et flasher le firmware** :  
   ```bash  
   idf.py build flash monitor  
   ```  

##  **Utilisation**  
### **1. Suivi des logs en temps réel**  
Une fois le module démarré, connectez-vous au moniteur série pour voir les logs en temps réel :  
```bash  
idf.py monitor  
```  

### **2. Tester la communication MQTT → LoRa**  
Envoyer un message sur le topic `kikimou` via Mosquitto :  
```bash  
mosquitto_pub -h test.mosquitto.org -t "kikimou" -m "Message test LoRa"  
```  
- L’ESP32 reçoit ce message et le transmet via LoRa.  
- Un autre module LoRa recevant ce message peut le traiter et répondre.  

### **3. Tester la communication LoRa → MQTT**  
Si un message est reçu via LoRa, il est automatiquement publié sur le topic `kikidur`. Vous pouvez afficher ce message avec :  
```bash  
mosquitto_sub -h test.mosquitto.org -t "kikidur"  
```  
