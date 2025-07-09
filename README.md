# OpenFields Project

**Offline-first, open-source IoT platform for agriculture in developing regions.**  
OpenFields is designed for resilience and autonomy — enabling farms to monitor, automate, and share data without requiring internet connectivity.

---

## 🌾 Project Vision

OpenFields empowers small-scale and remote farms in under-resourced areas with a reliable, low-cost alternative to commercial smart farming. Built with a focus on **local-first design**, it bridges the technology gap with robust, battery-efficient nodes, LoRa mesh networking, and community collaboration.

---
## Ethics & Philosophy

OpenFieldsProject is driven by a commitment to openness, equity, and sustainability. This is a non-profit, community-focused initiative built to serve, not sell.

- **Non-profit values:** Fully open-source with no paywalls or commercial agendas.  
- **User-centered design:** Created specifically for users in developing regions, designed to work offline on low-cost hardware.  
- **Correctness over cosmetics:** Accuracy and reliability are paramount because real-world agricultural decisions depend on trustworthy data.  
- **Sustainability:** Energy-efficient, low-bandwidth, and minimal dependencies ensure practical and responsible operation.  
- **Community collaboration:** Contributions are welcome from everyone — developers, farmers, researchers — to build meaningful tools together.

---

## 🌍 Key Features

- **Offline-first operation** — no reliance on internet access
- **LoRa + Wi-Fi hybrid networking** (inter-farm syncs over LoRa mesh during off-peak hours)
- **Secure AES-encrypted communications** between nodes and gateway
- **GPS-based time synchronization** using 1Hz pulse and UTC parsing
- **Fixed-cost hardware** with open-source C firmware (NASA⁽¹⁰⁾ style)
- **MQTT-based server** for data ingestion, processing, and dissemination
- **Web-based UI** (served from local NUC/AP) — no mobile app required
- **Human-readable MQTT topics and plain text payloads**
- **Energy-efficient design** — nodes sleep intelligently and wake for critical tasks
- **Modular architecture** — sensors, relays, weather, irrigation, etc.
- **Manual sync options** for historical weather or river data

---

## 🧱 Architecture Overview
[ Sensor Node ]
|
[ AES Encrypted LoRa ]
|
[ Gateway (NUC/AP, MQTT Broker) ]
|
[ Local Services ]
├── Time Sync (via GPS)
├── Input Handlers
├── Data Processing
├── Dissemination via MQTT
├── Post Process with RRDTOOL
└── Web UI (hosted locally)


> 🌐 Nodes connect via local Wi-Fi **or** LoRa — no internet needed.

---

## 🚀 Getting Started (Server Side)

1. **Install Debian (or similar) on your NUC/local server**
2. **Install dependencies**:
   - `mosquitto` (MQTT broker)
   - `rrdtool`, `mariadb`, for data storage handling
   - `apache2` (or preferred web server)
3. **Set up local-only MQTT topics** (e.g. `raw/nmea/gps`, `system/datetime/epoch`)
4. **Deploy post-processing scripts** (written in C)
5. **Set up LoRa transmitter services** to respect regional ISM duty cycles
6. **Secure system** using firewall and localhost MQTT access only

📄 Full install guide coming soon...

---

## 🤝 Community-Centric Design

- Meant to be **run and owned by local communities**
- No cloud, no hidden fees, no external dependencies
- **Shared water resources and river level data** synchronized at night
- Sync window (e.g. 2–3am) allows farms to **share and receive** vital info

---

## 📜 License

This project is licensed under the **GNU General Public License v3.0 (GPLv3)**.  
See the [`LICENSE`](LICENSE) file for more details.

---

## 🪴 Contributing

OpenFields is at an early stage and welcomes contributors — especially from:

- Agricultural communities
- Developers in developing regions
- Embedded systems engineers
- Localization and UX experts

📬 Reach out or open an issue to get involved.

> 🌐 Website: [https://openfieldsproject.org](https://openfieldsproject.org)  
> 📧 Contact: openfieldsproject@proton.me
---

## 📡 Disclaimer

OpenFields is designed for **resilience, privacy, and autonomy**.  
It is **not** intended for cloud-based, commercial agriculture — but for empowering those who need it most.

---

_“A mesh of communities, people, and technology — rooted in shared growth.”_


