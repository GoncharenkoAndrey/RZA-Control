#include <windows.h>
#include "Settings.h";
Settings *Settings::instance = 0;
Settings::Settings() {
	port = new wchar_t[5];
	memcpy(port, "COM1", 5 * sizeof(wchar_t));
	speed = 19200;
}
void Settings::setPort(wchar_t* p) {
	memcpy(port, p, 5 * sizeof(wchar_t));
}
void Settings::setSpeed(int s) {
	speed = s;
}
wchar_t *Settings::getPort() {
	return port;
}
int Settings::getSpeed() {
	return speed;
}