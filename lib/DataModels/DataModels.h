#ifndef DATAMODELS_H
#define DATAMODELS_H

#include <Arduino.h>
#include <vector>

// Structure for a single Alertzy account
struct AlertzyAccount {
	String name; // e.g., "My Phone"
	String key;
};

// Structure for a single phase within a multi-phase timer
struct TimerPhase {
	String name;
	uint32_t duration_seconds; // Use uint32_t for H/M/S
	uint8_t sound_track; // MP3 track number (1-255)
	std::vector<uint8_t> alertzy_key_indices; // Indices of keys to notify
};

// Structure for a complete custom timer routine
struct CustomTimer {
	String name;
	std::vector<TimerPhase> phases;
};

#endif
