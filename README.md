# 🔌 REX-47 Firmware

> **Repository `02`** · Embedded C/C++ firmware for ESP32 and other microcontrollers controlling low-level hardware, motors, and sensors.

![Platform](https://img.shields.io/badge/Platform-ESP32-blue)
![Language](https://img.shields.io/badge/Language-C%2B%2B-00599C?logo=c%2B%2B)
![Status](https://img.shields.io/badge/Status-Active-green)

---

## 📋 Table of Contents

- [Overview](#-what-is-this-repository)
- [Architecture](#-architecture)
- [Features](#-features)
- [Getting Started](#-getting-started)
- [Project Structure](#-project-structure)
- [Deployment](#-deployment)
- [Related Repositories](#-related-repositories)

---

## 🧭 What Is This Repository?

This repository contains the **REX-47 Firmware** for the REX-47 platform. Embedded C/C++ firmware for ESP32 and other microcontrollers controlling low-level hardware, motors, and sensors.

**Key Highlights:**
- ✅ Standardized integration with the broader REX-47 ecosystem
- ✅ Modular, scalable architecture
- ✅ Comprehensive environment configuration
- ✅ Dockerized for reliable deployment

---

## 🏗️ Architecture

### Directory Structure

```
02-rex-firmware/
├── README.md
```

---

## 🎨 Features

| Feature | Description |
|---------|-------------|
| **Core Functionality** | Specific implementation of the service domain. |
| **Integration** | Seamless connectivity with other REX-47 modules. |
| **Configuration** | Environment-variable driven settings. |
| **Containerization** | Production-ready Docker configuration. |

---

## 🚀 Getting Started

### Prerequisites

- Modern runtime environment appropriate for this service (Node.js, Python, Flutter, etc.)
- Docker and Docker Compose (recommended for running the full stack)

### Installation

```bash
# Clone the repository
git clone https://github.com/thathsarabandara/02-rex-firmware.git
cd 02-rex-firmware

# Install dependencies (if applicable)
# npm install | pip install -r requirements.txt | flutter pub get
```

### Configuration

Create a `.env` file based on the provided `.env.example`:

```bash
cp .env.example .env
```

Ensure all secret keys and port configurations are set correctly.

---

## 📦 Deployment

### Using Docker Compose

This service is integrated into the REX-47 multi-container architecture. To run this service using Docker:

```bash
docker compose up --build
```

---

## 🔗 Related Repositories

- [01-rex-architecture](../01-rex-architecture) — REX-47 System Architecture
- [02-rex-firmware](../02-rex-firmware) — REX-47 Firmware
- [03-rex-web-dashbaord](../03-rex-web-dashbaord) — REX-47 Web Dashboard
- [04-rex-mobile-app](../04-rex-mobile-app) — REX-47 Mobile App
- [05-rex-api-gateway](../05-rex-api-gateway) — REX-47 API Gateway
- [06-rex-auth-service](../06-rex-auth-service) — REX-47 Auth Service
- [07-rex-robot-service](../07-rex-robot-service) — REX-47 Robot Service
- [08-rex-telemetry-service](../08-rex-telemetry-service) — REX-47 Telemetry Service
- [09-rex-sensor-fusion](../09-rex-sensor-fusion) — REX-47 Sensor Fusion
- [10-rex-navigation-engine](../10-rex-navigation-engine) — REX-47 Navigation Engine
- [11-rex-vision-ai](../11-rex-vision-ai) — REX-47 Vision AI
- [12-rex-event-engine](../12-rex-event-engine) — REX-47 Event Engine
- [13-rex-notification-engine](../13-rex-notification-engine) — REX-47 Notification Engine
- [14-rex-voice-assistant](../14-rex-voice-assistant) — REX-47 Voice Assistant
- [15-rex-agent-runtime](../15-rex-agent-runtime) — REX-47 Agent Runtime
- [16-rex-memory-engine](../16-rex-memory-engine) — REX-47 Memory Engine
- [17-rex-devops-infras](../17-rex-devops-infras) — REX-47 DevOps & Infrastructure
