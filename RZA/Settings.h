#pragma once
class Settings {
	static Settings *instance;
private:
	wchar_t* port;
	int speed;
	Settings();
public:
	static Settings *getInstance() {
		if (!instance) {
			instance = new Settings();
		}
		return instance;
	}
	void setPort(wchar_t* p);
	void setSpeed(int s);
	wchar_t* getPort();
	int getSpeed();
};