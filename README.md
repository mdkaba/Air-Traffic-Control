# Air Traffic Control System (C++ / QNX)
<img width="808" height="681" alt="image" src="https://github.com/user-attachments/assets/913eafa2-1d92-4444-ac81-e2ff90594bbf" />

This project implements a simplified but realistic **Air Traffic Control (ATC)** system built in **C++** and designed to run on the **QNX real-time operating system**.  
The system simulates aircraft, radar updates, operator commands, and collision-risk detection using multi-process communication and periodic real-time updates.

## ✈️ Overview

The system models the core responsibilities of an ATC center:

- Tracking aircraft positions in real time  
- Exchanging messages between multiple processes (radar, aircraft, operator console)  
- Predicting potential conflicts based on velocity vectors  
- Logging system status and collision warnings  
- Responding to operator commands (speed, altitude, position, prediction window)

## 🧩 System Architecture

<img width="1000" height="666" alt="atc_uml_diagram" src="https://github.com/user-attachments/assets/bf3c4a6d-9d9e-4e3a-9b47-21a3c23a4758" />

### 1. ComputerSystem (Core Coordinator)
- Maintains the shared, real-time state of all aircraft  
- Receives radar updates and operator commands  
- Stores aircraft objects in memory  
- Triggers collision detection at regular intervals  
- Logs system events and aircraft lists  

### 2. Radar Subsystem
- Periodically calculates updated aircraft positions  
- Sends updates to the ComputerSystem  
- Simulates real radar sweeps using timers  

### 3. Aircraft Processes
- One process per aircraft  
- Stores speed, altitude, heading, and position  
- Responds to operator commands routed through CommSystem  

### 4. Operator Console
- Allows the operator to:
  - Select an aircraft  
  - Change altitude, speed, or heading  
  - Modify the conflict prediction time window  
- Sends commands through the CommSystem server  

### 5. CommSystem
- Central message-routing server  
- Receives operator commands  
- Delivers them to the target aircraft process  
- Isolates communication logic from core ATC logic  

### 6. Display System
- Prints:
  - Aircraft lists  
  - System logs  
  - Collision warnings (“WARNING”, “IMMINENT”)  
- Updated periodically by the ComputerSystem  

## 🔍 Collision Detection

The system includes a predictive collision-risk algorithm:

- Uses aircraft **positions** and **velocity vectors**  
- Computes predicted separation over a configurable horizon  
- Default window: **180 seconds**  
- Produces:
  - **WARNING** alerts  
  - **IMMINENT** alerts  
- Logs each detection event with timestamps  

## 📡 Inter-Process Communication (QNX)

- `MsgSend()` / `MsgReceive()` for process-to-process commands  
- Pulses and timers for periodic updates  
- Mutex-protected shared data  
- Channel-based message routing  

## 📁 Project Structure
```
Air-Traffic-Control/
├── src/
├── Aircraft/
└── build/
```
## 🛠️ Build & Run Instructions

### Requirements
- QNX SDP  
- QCC or GCC toolchain  

### Build
```bash
make
```

### Run
```bash
./build/atc
```
## 🧪 Testing & Scalability

- Stable under increasing aircraft load  
- Real-time updates maintained  
- Constraints approached at ~60 aircrafts  

## 👥 Contributors
- Mamadou Kaba  
- Maïka Trépanier 
- Lifu Zhang 
