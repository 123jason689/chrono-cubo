#include "StorageManager.h"
#include <ArduinoJson.h>

StorageManager::StorageManager(Preferences* prefs) : preferences(prefs) {}

std::vector<AlertzyAccount> StorageManager::loadAlertzyAccounts() {
	std::vector<AlertzyAccount> accounts;
	if (!preferences) return accounts;
	preferences->begin("storage", true, "nvs");
	String json = preferences->getString("alertzy_accounts", "");
	preferences->end();
	if (json.length() == 0) return accounts;

	DynamicJsonDocument doc(4096);
	auto err = deserializeJson(doc, json);
	if (err) return accounts;
	if (!doc.is<JsonArray>()) return accounts;

	for (JsonVariant v : doc.as<JsonArray>()) {
		AlertzyAccount acc;
		acc.name = v["name"].as<String>();
		acc.key = v["key"].as<String>();
		accounts.push_back(acc);
	}
	return accounts;
}

void StorageManager::saveAlertzyAccounts(const std::vector<AlertzyAccount>& accounts) {
	DynamicJsonDocument doc(4096);
	JsonArray arr = doc.to<JsonArray>();
	for (const auto& acc : accounts) {
		JsonObject o = arr.add<JsonObject>();
		o["name"] = acc.name;
		o["key"] = acc.key;
	}
	String json;
	serializeJson(doc, json);
	if (!preferences) return;
	preferences->begin("storage", false, "nvs");
	preferences->putString("alertzy_accounts", json);
	preferences->end();
}

std::vector<CustomTimer> StorageManager::loadCustomTimers() {
	std::vector<CustomTimer> timers;
	if (!preferences) return timers;
	preferences->begin("storage", true, "nvs");
	String json = preferences->getString("custom_timers", "");
	preferences->end();
	if (json.length() == 0) return timers;

	DynamicJsonDocument doc(16384);
	auto err = deserializeJson(doc, json);
	if (err) return timers;
	if (!doc.is<JsonArray>()) return timers;

	for (JsonVariant t : doc.as<JsonArray>()) {
		CustomTimer timer;
		timer.name = t["name"].as<String>();
		JsonArray phases = t["phases"].as<JsonArray>();
		for (JsonVariant p : phases) {
			TimerPhase phase;
			phase.name = p["name"].as<String>();
			phase.duration_seconds = p["duration_seconds"].as<uint32_t>();
			phase.sound_track = p["sound_track"].as<uint8_t>();
			for (JsonVariant idx : p["alertzy_key_indices"].as<JsonArray>()) {
				phase.alertzy_key_indices.push_back(idx.as<uint8_t>());
			}
			timer.phases.push_back(phase);
		}
		timers.push_back(timer);
	}
	return timers;
}

void StorageManager::saveCustomTimers(const std::vector<CustomTimer>& timers) {
	DynamicJsonDocument doc(16384);
	JsonArray arr = doc.to<JsonArray>();
	for (const auto& timer : timers) {
		JsonObject to = arr.add<JsonObject>();
		to["name"] = timer.name;
		JsonArray phases = to.createNestedArray("phases");
		for (const auto& phase : timer.phases) {
			JsonObject po = phases.add<JsonObject>();
			po["name"] = phase.name;
			po["duration_seconds"] = phase.duration_seconds;
			po["sound_track"] = phase.sound_track;
			JsonArray keys = po.createNestedArray("alertzy_key_indices");
			for (auto idx : phase.alertzy_key_indices) keys.add(idx);
		}
	}
	String json;
	serializeJson(doc, json);
	if (!preferences) return;
	preferences->begin("storage", false, "nvs");
	preferences->putString("custom_timers", json);
	preferences->end();
}

std::vector<Alarm> StorageManager::loadAlarms() {
	std::vector<Alarm> alarms;
	if (!preferences) return alarms;
	preferences->begin("storage", true, "nvs");
	String json = preferences->getString("alarms", "");
	preferences->end();
	if (json.length() == 0) return alarms;

	DynamicJsonDocument doc(1024);
	auto err = deserializeJson(doc, json);
	if (err) return alarms;
	if (!doc.is<JsonArray>()) return alarms;

	for (JsonVariant v : doc.as<JsonArray>()) {
		Alarm a;
		a.hour = v["hour"].as<uint8_t>();
		a.minute = v["minute"].as<uint8_t>();
		a.enabled = v["enabled"].as<bool>();
		a.sound_track = v["sound_track"].as<uint8_t>();
		alarms.push_back(a);
	}
	return alarms;
}

void StorageManager::saveAlarms(const std::vector<Alarm>& alarms) {
	if (!preferences) return;
	DynamicJsonDocument doc(1024);
	JsonArray arr = doc.to<JsonArray>();
	for (const auto& a : alarms) {
		JsonObject o = arr.add<JsonObject>();
		o["hour"] = a.hour;
		o["minute"] = a.minute;
		o["enabled"] = a.enabled;
		o["sound_track"] = a.sound_track;
	}
	String json;
	serializeJson(doc, json);
	preferences->begin("storage", false, "nvs");
	preferences->putString("alarms", json);
	preferences->end();
}
