#include "Config.hpp"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QKeyEvent>

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
	MSLLHOOKSTRUCT *p = (MSLLHOOKSTRUCT *)lParam;
	if (p->flags & LLMHF_INJECTED)
		return CallNextHookEx(nullptr, nCode, wParam, lParam);

	if (nCode == HC_ACTION && instance->m_isRecording) {
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

		inputs[1] = inputs[0];
		inputs[1].mi.dwFlags |=
		    (vk == VK_LBUTTON ? MOUSEEVENTF_LEFTUP : MOUSEEVENTF_RIGHTUP);
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