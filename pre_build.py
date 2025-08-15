Import("env")
import os
import sys
from pathlib import Path

def generate_config_h():
    """Generate config.h from .env.local file"""
    
    # Get project directory
    project_dir = Path(env.get("PROJECT_DIR"))
    env_file = project_dir / ".env.local"
    config_file = project_dir / "include" / "configs.h"
    
    # Create include directory if it doesn't exist
    config_file.parent.mkdir(exist_ok=True)
    
    # Default values
    config_values = {
        'OLED_I2C_ADDRESS': '0x3C',
        'OLED_SDA_PIN': '8',
        'OLED_SCL_PIN': '9',
        'OLED_RESET_PIN': '-1',
        'SCREEN_WIDTH': '128',
        'SCREEN_HEIGHT': '64',
        'POT_X_PIN': '0',
        'POT_Y_PIN': '1',
        'BTN_SELECT': '3',
        'POT_DEADZONE': '100',
        'POT_CENTER': '2048',
        'POT_SENSITIVITY': '3',
        'MOVE_DELAY': '150',
        'SLEEP_DURATION_SECONDS': '300',
        'WIFI_TIMEOUT_MS': '15000'
    }
    
    # Read .env.local if it exists
    if env_file.exists():
        print(f"Reading configuration from {env_file}")
        with open(env_file, 'r') as f:
            for line in f:
                line = line.strip()
                if line and not line.startswith('#') and '=' in line:
                    key, value = line.split('=', 1)
                    config_values[key.strip()] = value.strip()
    else:
        print(f"No .env.local found, using defaults")
    
    # Generate config.h
    config_content = '''#ifndef CONFIG_H
#define CONFIG_H

// ========================================
// WiFi Display Module Configuration
// ========================================
// This file is auto-generated from .env.local
// Do not edit directly - edit .env.local instead

// OLED Display I2C Configuration
#define OLED_I2C_ADDRESS ''' + config_values['OLED_I2C_ADDRESS'] + '''
#define OLED_SDA_PIN ''' + config_values['OLED_SDA_PIN'] + '''
#define OLED_SCL_PIN ''' + config_values['OLED_SCL_PIN'] + '''
#define OLED_RESET_PIN ''' + config_values['OLED_RESET_PIN'] + '''

// Display Settings
#define SCREEN_WIDTH ''' + config_values['SCREEN_WIDTH'] + '''
#define SCREEN_HEIGHT ''' + config_values['SCREEN_HEIGHT'] + '''

// Input Control Pins
#define POT_X_PIN ''' + config_values['POT_X_PIN'] + '''
#define POT_Y_PIN ''' + config_values['POT_Y_PIN'] + '''
#define BTN_SELECT ''' + config_values['BTN_SELECT'] + '''

// Hardware Settings
#define POT_DEADZONE ''' + config_values['POT_DEADZONE'] + '''
#define POT_CENTER ''' + config_values['POT_CENTER'] + '''
#define POT_SENSITIVITY ''' + config_values['POT_SENSITIVITY'] + '''
#define MOVE_DELAY ''' + config_values['MOVE_DELAY'] + '''

// Power Management
#define SLEEP_DURATION_SECONDS ''' + config_values['SLEEP_DURATION_SECONDS'] + '''
#define WIFI_TIMEOUT_MS ''' + config_values['WIFI_TIMEOUT_MS'] + '''

// Special Characters for KeyInput
#define REMOVE_CHAR 127
#define LEFT_CHAR 128
#define RIGHT_CHAR 129
#define SPACE_CHAR 130

#endif // CONFIG_H
'''
    
    # Write the config file
    with open(config_file, 'w') as f:
        f.write(config_content)
    
    print(f"Generated {config_file}")

def pre_build_callback(source, target, env):
    """Pre-build callback"""
    print("Running pre-build configuration...")
    generate_config_h()

# Generate config immediately
generate_config_h()

# Also add it as a pre-action for future builds
env.AddPreAction("buildprog", pre_build_callback)
