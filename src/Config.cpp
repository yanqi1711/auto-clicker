#include "Config.hpp"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QKeyEvent>
#include <QMap>

Config *Config::instance = nullptr;

QString vkToName(int vk) {
	if (vk == VK_LBUTTON)
		return "LeftClick";
	if (vk == VK_RBUTTON)
		return "RightClick";
	if (vk == VK_MBUTTON)
		return "MidClick";

	UINT scanCode = MapVirtualKey(vk, MAPVK_VK_TO_VSC);
	if (vk >= VK_PRIOR && vk <= VK_HELP)
		scanCode |= 0x100;

	WCHAR name[64];
	if (GetKeyNameTextW(scanCode << 16, name, 64) > 0) {
		return QString::fromWCharArray(name);
	}
	return QString("Key_%1").arg(vk);
}

int nameToVk(const QString &name) {
	static const QMap<QString, int> keyMap = {
	    {"LeftClick", VK_LBUTTON},   {"RightClick", VK_RBUTTON},
	    {"MidClick", VK_MBUTTON},    {"LeftMouseBtn", VK_LBUTTON},
	    {"RightMouseBtn", VK_RBUTTON},
	    {"MiddleMouseBtn", VK_MBUTTON},
	    {"F1", VK_F1},               {"F2", VK_F2},
	    {"F3", VK_F3},               {"F4", VK_F4},
	    {"F5", VK_F5},               {"F6", VK_F6},
	    {"F7", VK_F7},               {"F8", VK_F8},
	    {"F9", VK_F9},               {"F10", VK_F10},
	    {"F11", VK_F11},             {"F12", VK_F12},
	};

	if (name.size() == 1) {
		SHORT vk = VkKeyScanW(name.at(0).unicode());
		if (vk != -1)
			return LOBYTE(vk);
	}

	return keyMap.value(name, 0);
}

Config::Config(QObject *parent) : QObject(parent) {
	instance = this;

	m_clickTimer = new QTimer(this);
	connect(m_clickTimer, &QTimer::timeout, this, &Config::performAction);

	m_saveTimer = new QTimer(this);
	m_saveTimer->setSingleShot(true);
	connect(m_saveTimer, &QTimer::timeout, this, &Config::saveConfig);

	QFile file(m_fileName);
	if (file.open(QIODevice::ReadOnly)) {
		m_data = QJsonDocument::fromJson(file.readAll()).object();
		file.close();
	}

	bool configMigrated = false;
	if (!m_data.contains("hotkeyVk")) {
		const QString oldHotkey = m_data["hotkey"].toString("F1");
		m_data["hotkeyVk"] = nameToVk(oldHotkey);
		m_data["hotkeyName"] = vkToName(m_data["hotkeyVk"].toInt(VK_F1));
		configMigrated = true;
	}
	if (!m_data.contains("simulateKeyVk")) {
		const QString oldSimulateKey = m_data["simulateKey"].toString("LeftClick");
		m_data["simulateKeyVk"] = nameToVk(oldSimulateKey);
		m_data["simulateKeyName"] =
		    vkToName(m_data["simulateKeyVk"].toInt(VK_LBUTTON));
		configMigrated = true;
	}
	if (!m_data.contains("interval")) {
		m_data["interval"] = 100;
		configMigrated = true;
	}

	if (m_data["hotkeyVk"].toInt() == 0) {
		m_data["hotkeyVk"] = VK_F1;
		m_data["hotkeyName"] = vkToName(VK_F1);
		configMigrated = true;
	}
	if (m_data["simulateKeyVk"].toInt() == 0) {
		m_data["simulateKeyVk"] = VK_LBUTTON;
		m_data["simulateKeyName"] = vkToName(VK_LBUTTON);
		configMigrated = true;
	}

	if (configMigrated)
		saveConfig();

	startHook();
}

Config::~Config() {
	if (m_kbdHook)
		UnhookWindowsHookEx(m_kbdHook);
	if (m_mouseHook)
		UnhookWindowsHookEx(m_mouseHook);
}

Config *Config::getInstance() {
	return instance;
}

LRESULT CALLBACK Config::LowLevelKeyboardProc(int nCode, WPARAM wParam,
                                              LPARAM lParam) {
	if (!instance)
		return CallNextHookEx(nullptr, nCode, wParam, lParam);

	if (nCode == HC_ACTION) {
		KBDLLHOOKSTRUCT *p = (KBDLLHOOKSTRUCT *)lParam;

		if (p->vkCode == VK_ESCAPE && (GetKeyState(VK_SHIFT) & 0x8000)) {
			if (instance->m_isRunning)
				instance->toggleAutoClick();
			return CallNextHookEx(nullptr, nCode, wParam, lParam);
		}

		if (instance->m_isRecording) {
			QString name = vkToName(p->vkCode);
			if (instance->m_recordingTarget == "hotkey")
				instance->setHotkey(name, p->vkCode);
			else
				instance->setSimulateKey(name, p->vkCode);

			instance->setIsRecording(false);
			return 1;
		}

		if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
			if (p->vkCode == instance->m_data["hotkeyVk"].toInt()) {
				instance->toggleAutoClick();
				return 1;
			}
		}
	}
	return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

LRESULT CALLBACK Config::LowLevelMouseProc(int nCode, WPARAM wParam,
                                           LPARAM lParam) {
	if (!instance)
		return CallNextHookEx(nullptr, nCode, wParam, lParam);

	if (nCode != HC_ACTION)
		return CallNextHookEx(nullptr, nCode, wParam, lParam);

	MSLLHOOKSTRUCT *p = (MSLLHOOKSTRUCT *)lParam;
	if (p->flags & LLMHF_INJECTED)
		return CallNextHookEx(nullptr, nCode, wParam, lParam);

	if (instance->m_isRecording) {
		int vk = 0;
		if (wParam == WM_LBUTTONDOWN)
			vk = VK_LBUTTON;
		else if (wParam == WM_RBUTTONDOWN)
			vk = VK_RBUTTON;
		else if (wParam == WM_MBUTTONDOWN)
			vk = VK_MBUTTON;

		if (vk != 0) {
			QString name = vkToName(vk);
			if (instance->m_recordingTarget == "hotkey")
				instance->setHotkey(name, vk);
			else
				instance->setSimulateKey(name, vk);
			instance->setIsRecording(false);
			return 1;
		}
	}
	return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

void Config::performAction() {
	int vk = m_data["simulateKeyVk"].toInt();
	if (vk == 0)
		return;

	INPUT inputs[2] = {0};
	if (vk == VK_LBUTTON || vk == VK_RBUTTON || vk == VK_MBUTTON) {
		inputs[0].type = INPUT_MOUSE;
		if (vk == VK_LBUTTON)
			inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
		else if (vk == VK_RBUTTON)
			inputs[0].mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
		else if (vk == VK_MBUTTON)
			inputs[0].mi.dwFlags = MOUSEEVENTF_MIDDLEDOWN;

		inputs[1] = inputs[0];
		if (vk == VK_LBUTTON)
			inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;
		else if (vk == VK_RBUTTON)
			inputs[1].mi.dwFlags = MOUSEEVENTF_RIGHTUP;
		else
			inputs[1].mi.dwFlags = MOUSEEVENTF_MIDDLEUP;
	} else {
		inputs[0].type = INPUT_KEYBOARD;
		inputs[0].ki.wVk = (WORD)vk;
		inputs[1] = inputs[0];
		inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
	}
	SendInput(2, inputs, sizeof(INPUT));
}

void Config::setHotkey(const QString &name, int vk) {
	if (m_data["hotkeyVk"].toInt() != vk) {
		m_data["hotkeyName"] = name;
		m_data["hotkeyVk"] = vk;
		emit hotkeyChanged();
		markDirty();
	}
}

void Config::setSimulateKey(const QString &name, int vk) {
	if (m_data["simulateKeyVk"].toInt() != vk) {
		m_data["simulateKeyName"] = name;
		m_data["simulateKeyVk"] = vk;
		emit simulateKeyChanged();
		markDirty();
	}
}

void Config::setInterval(int v) {
	v = qMax(1, v);

	if (interval() != v) {
		m_data["interval"] = v;
		emit intervalChanged();
		if (m_isRunning)
			m_clickTimer->start(v);
		markDirty();
	}
}

void Config::startRecording(QString target) {
	m_recordingTarget = target;
	setIsRecording(true);
}

void Config::setIsRecording(bool v) {
	if (m_isRecording != v) {
		m_isRecording = v;
		emit isRecordingChanged();
	}
}

void Config::markDirty() {
	m_saveTimer->start(1000);
}

void Config::saveConfig() {
	QDir().mkpath(QFileInfo(m_fileName).absolutePath());
	QFile file(m_fileName);
	if (file.open(QIODevice::WriteOnly)) {
		file.write(QJsonDocument(m_data).toJson());
		file.close();
		qDebug() << "Config saved.";
	}
}

void Config::startHook() {
	m_kbdHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc,
	                             GetModuleHandle(nullptr), 0);
	m_mouseHook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc,
	                               GetModuleHandle(nullptr), 0);
}

void Config::stopAutoClick() {
	if (m_isRunning) {
		m_isRunning = false;
		m_clickTimer->stop();
		emit isRunningChanged();
	}
}

void Config::toggleAutoClick() {
	if (m_isRunning) {
		stopAutoClick();
	} else {
		m_isRunning = true;
		m_clickTimer->start(interval());
		emit isRunningChanged();
	}
}
