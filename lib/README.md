# Chrono-Cubo Libraries

This directory contains the custom libraries for the Chrono-Cubo smart timer project.

## Core Libraries

### KeyInput
- **Purpose**: Handles user input via potentiometers and button
- **Features**: 
  - Analog input processing with debouncing
  - On-screen keyboard for password entry
  - Movement detection and button press handling
- **Files**: `KeyInput.h`, `KeyInput.cpp`

### WiFiSelector
- **Purpose**: Manages WiFi network scanning, selection, and connection
- **Features**:
  - Network scanning and display
  - Credential storage and retrieval
  - Connection management with timeout handling
- **Files**: `WiFiSelector.h`, `WiFiSelector.cpp`

### ScrollingText
- **Purpose**: Displays long text strings with smooth scrolling
- **Features**:
  - Configurable scroll speed and pause duration
  - Smooth pixel-based scrolling
  - Loop and direction control
- **Files**: `ScrollingText.h`, `ScrollingText.cpp`

## Timer Libraries

### TimeManager
- **Purpose**: Handles NTP time synchronization and RTC management
- **Features**:
  - NTP server synchronization
  - RTC time setting and retrieval
  - Time formatting and display
  - Timezone configuration
- **Files**: `TimeManager.h`, `TimeManager.cpp`

### StateMachine
- **Purpose**: Manages application states and menu navigation
- **Features**:
  - State transitions and management
  - Menu system with navigation
  - State-specific handling
- **Files**: `StateMachine.h`, `StateMachine.cpp`

### SingleTimer
- **Purpose**: Implements single-use timer functionality
- **Features**:
  - Timer setup with minutes/seconds input
  - Countdown display with progress bar
  - Timer completion alerts
  - Pause/resume functionality
- **Files**: `SingleTimer.h`, `SingleTimer.cpp`

### MultiTimer
- **Purpose**: Implements multi-phase timer with preset routines
- **Features**:
  - Preset routines (Pomodoro, Workout, Study)
  - Phase transitions with notifications
  - Progress tracking per phase
  - Routine completion handling
- **Files**: `MultiTimer.h`, `MultiTimer.cpp`

### AlarmClock
- **Purpose**: Implements sleep alarm functionality
- **Features**:
  - Alarm time setting (hour/minute)
  - Real-time alarm checking
  - Alarm triggering with visual alerts
  - Time until alarm display
- **Files**: `AlarmClock.h`, `AlarmClock.cpp`

## Usage

All libraries are designed to work together through the main application in `src/main.cpp`. The libraries follow a modular design pattern where each handles a specific aspect of the timer functionality.

### Key Features:
- **Modular Design**: Each library handles a specific functionality
- **Consistent Interface**: All libraries use similar patterns for initialization and operation
- **Error Handling**: Robust error handling and fallback mechanisms
- **Memory Efficient**: Optimized for ESP32-C3 memory constraints
- **User-Friendly**: Intuitive UI with clear navigation and feedback

### Integration:
The libraries are integrated through the main application which:
1. Initializes all components
2. Manages state transitions
3. Handles user input routing
4. Coordinates timer operations
5. Manages display updates

## Configuration

All hardware-specific configurations are managed through `include/configs.h` which is auto-generated from `.env.local` by the pre-build script.
