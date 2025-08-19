# Chrono-Cubo - Smart Multi-Purpose Timer

A versatile, reliable, and user-friendly smart timer based on the ESP32-C3 Super Mini with a "cute and innovative" brand image.

## Features

### 🕐 Core Timer Functions
- **Single Timer**: Set custom countdown timers with minutes and seconds
- **Multi-Phase Timer**: Pre-configured routines with automatic phase transitions
- **Sleep Alarm**: Set daily alarms with visual and audio alerts

### 📱 User Interface
- **OLED Display**: 128x64 pixel display with clear, intuitive interface
- **Analog Controls**: Smooth potentiometer-based navigation
- **Button Interface**: Simple button for selection and confirmation
- **Visual Feedback**: Progress bars, flashing alerts, and status indicators

### 🌐 Connectivity
- **WiFi Management**: Easy network setup with on-screen keyboard
- **NTP Synchronization**: Automatic time synchronization
- **RTC Backup**: Reliable timekeeping even without WiFi

### ⚙️ Preset Routines
- **Pomodoro Technique**: 25-minute work sessions with 5-minute breaks
- **Workout Timer**: Warmup, exercise, and cooldown phases
- **Study Timer**: Focus, review, and break intervals

## Hardware Requirements

- ESP32-C3 Super Mini
- 128x64 OLED Display (SSD1306)
- DS3231 RTC Module
- 2x Potentiometers (X and Y axis)
- 1x Push Button
- Power supply

## Pin Configuration

Configure pins in `.env.local`:
```
OLED_SDA_PIN=8
OLED_SCL_PIN=9
OLED_I2C_ADDRESS=0x3C
POT_X_PIN=0
POT_Y_PIN=1
BTN_SELECT=2
```

## Installation

1. Clone the repository
2. Install PlatformIO
3. Create `.env.local` with your configuration
4. Build and upload to your ESP32-C3

```bash
pio run -t upload
```

## Usage

### Initial Setup
1. Power on the device
2. Follow on-screen prompts to connect to WiFi
3. Device will sync time automatically
4. Press button to access main menu

### Navigation
- **Y-Potentiometer**: Navigate menu items
- **X-Potentiometer**: Adjust values in setup screens
- **Button**: Select/confirm actions

### Timer Functions

#### Single Timer
1. Select "Single Timer" from main menu
2. Use Y-pot to switch between minutes/seconds
3. Use X-pot to adjust values
4. Press button to start countdown
5. Timer will alert when complete

#### Multi-Phase Timer
1. Select "Multi-Phase Timer" from main menu
2. Choose from available routines:
   - Pomodoro (25min work + 5min break)
   - Workout (5min warmup + 20min exercise + 5min cooldown)
   - Study (45min focus + 10min review + 15min break)
3. Press button to start routine
4. Device automatically transitions between phases

#### Sleep Alarm
1. Select "Sleep Alarm" from main menu
2. Set alarm time using potentiometers
3. Press button to confirm
4. Alarm will trigger at specified time
5. Press button to acknowledge alarm

## Development

### Project Structure
```
chrono-cubo/
├── src/main.cpp              # Main application
├── lib/                      # Custom libraries
│   ├── KeyInput/            # Input handling
│   ├── WiFiSelector/        # WiFi management
│   ├── TimeManager/         # Time synchronization
│   ├── StateMachine/        # Application states
│   ├── SingleTimer/         # Single timer logic
│   ├── MultiTimer/          # Multi-phase timer
│   └── AlarmClock/          # Alarm functionality
├── include/configs.h         # Auto-generated config
└── platformio.ini           # Build configuration
```

### Adding New Features
1. Create new library in `lib/` directory
2. Follow existing library patterns
3. Update state machine if needed
4. Add to main.cpp integration

## Future Enhancements

- Google Calendar integration
- Local network control
- Custom routine creation
- Multiple alarm support
- Sound alerts
- Battery optimization

## License

This project is open source and available under the MIT License.

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for bugs and feature requests.
