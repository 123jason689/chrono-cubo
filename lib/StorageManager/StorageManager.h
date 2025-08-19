#ifndef STORAGEMANAGER_H
#define STORAGEMANAGER_H

#include "DataModels.h"
#include <Preferences.h>
#include <vector>

class StorageManager {
private:
	Preferences* preferences;

public:
	StorageManager(Preferences* prefs);
	
	// Alertzy Key Management
	std::vector<AlertzyAccount> loadAlertzyAccounts();
	void saveAlertzyAccounts(const std::vector<AlertzyAccount>& accounts);

	// Custom Timer Management
	std::vector<CustomTimer> loadCustomTimers();
	void saveCustomTimers(const std::vector<CustomTimer>& timers);
};

#endif
