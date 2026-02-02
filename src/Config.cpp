#include "Config.hpp"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QKeyEvent>

Config *Config::instance = nullptr;
Config::Config(QObject *parent) : QObject(parent) {
	instance = this;
	m_clickTimer = new QTimer(this);
	connect(m_clickTimer, &QTimer::timeout, this, &Config::performAction);

	loadConfig();
	startHook(); // 初始化时启动钩子
}

Config::~Config() {
	if (m_hook)
		UnhookWindowsHookEx(m_hook);
	if (m_mouseHook)
		UnhookWindowsHookEx(m_mouseHook);
}

QString vkToName(int vk) {
	if (vk == VK_LBUTTON)
		return "LeftClick";
	if (vk == VK_RBUTTON)
		return "RightClick";
	if (vk >= VK_F1 && vk <= VK_F12)
		return "F" + QString::number(vk - VK_F1 + 1);
	if (vk == VK_SPACE)
		return "Space";
	if (vk == VK_ESCAPE)
		return "Esc";
	if (vk == VK_CONTROL || vk == VK_LCONTROL || vk == VK_RCONTROL)
		return "Ctrl";

	// 其他字符键
	UINT scanCode = MapVirtualKey(vk, MAPVK_VK_TO_VSC);
	WCHAR name[64];
	if (GetKeyNameTextW(scanCode << 16, name, 64) > 0) {
		return QString::fromWCharArray(name);
	}
	return QString("Key_%1").arg(vk);
}

// 核心：Windows 键盘消息回调
LRESULT CALLBACK Config::LowLevelKeyboardProc(int nCode, WPARAM wParam,
                                              LPARAM lParam) {
	MSLLHOOKSTRUCT *p = (MSLLHOOKSTRUCT *)lParam;
	if (p->flags & LLMHF_INJECTED)
		return CallNextHookEx(NULL, nCode, wParam, lParam);
	if (nCode == HC_ACTION &&
	    (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)) {
		KBDLLHOOKSTRUCT *p = (KBDLLHOOKSTRUCT *)lParam;

		// 将 VK Code 转换为字符串 (这里简单处理，实际可做更复杂的映射)
		QString keyName =
		    QString::fromStdWString(std::wstring(1, (wchar_t)p->vkCode));
		if (p->vkCode >= VK_F1 && p->vkCode <= VK_F12)
			keyName = "F" + QString::number(p->vkCode - VK_F1 + 1);

		// 场景 A: 正在录制
		// 在 LowLevelKeyboardProc 里的录制逻辑处：
		if (instance->m_isRecording) {
			QString keyName = vkToName(p->vkCode);
			if (instance->m_recordingTarget == "hotkey") {
				instance->setHotkey(keyName);
			} else {
				instance->setSimulateKey(keyName);
			}
			instance->setIsRecording(false);
			return 1;
		}

		// 场景 B: 按下了启动热键
		if (keyName == instance->hotkey()) {
			instance->toggleAutoClick();
			return 1; // 拦截，防止连点时误触发其他软件功能
		}
	}
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

LRESULT CALLBACK Config::LowLevelMouseProc(int nCode, WPARAM wParam,
                                           LPARAM lParam) {
	MSLLHOOKSTRUCT *p = (MSLLHOOKSTRUCT *)lParam;
	if (p->flags & LLMHF_INJECTED)
		return CallNextHookEx(NULL, nCode, wParam, lParam);
	if (nCode == HC_ACTION && instance->m_isRecording) {
		QString btnName = "";

		// 识别点击了哪个键
		if (wParam == WM_LBUTTONDOWN)
			btnName = "LeftClick";
		else if (wParam == WM_RBUTTONDOWN)
			btnName = "RightClick";
		else if (wParam == WM_MBUTTONDOWN)
			btnName = "MidClick"; // 滚轮按键

		if (!btnName.isEmpty()) {
			if (instance->m_recordingTarget == "hotkey") {
				instance->setHotkey(btnName);
			} else {
				instance->setSimulateKey(btnName);
			}
			instance->setIsRecording(false);
			return 1; // 拦截点击，防止录制时误触发 UI 按钮
		}
	}
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

void Config::startHook() {
	m_hook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc,
	                          GetModuleHandle(NULL), 0);
	m_mouseHook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc,
	                               GetModuleHandle(NULL), 0);
}

void Config::toggleAutoClick() {
	m_isRunning = !m_isRunning;
	if (m_isRunning) {
		m_clickTimer->start(interval());
	} else {
		m_clickTimer->stop();
	}
	emit isRunningChanged();
}

// 模拟点击逻辑
void Config::performAction() {
	QString action = simulateKey();
	INPUT input = {0};
	input.type = INPUT_MOUSE;

	if (action == "LeftClick") {
		// 模拟左键按下和弹起
		input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
		SendInput(1, &input, sizeof(INPUT));
		input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
		SendInput(1, &input, sizeof(INPUT));
	} else if (action == "RightClick") {
		input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
		SendInput(1, &input, sizeof(INPUT));
		input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
		SendInput(1, &input, sizeof(INPUT));
	} else if (action == "MidClick") {
		input.mi.dwFlags = MOUSEEVENTF_MIDDLEDOWN;
		SendInput(1, &input, sizeof(INPUT));
		input.mi.dwFlags = MOUSEEVENTF_MIDDLEUP;
		SendInput(1, &input, sizeof(INPUT));
	} else {
		// 模拟键盘：这里假设 simulateKey 存的是 VK 码
		// 实际开发建议写个映射表
		BYTE vk = (BYTE)action.at(0).toLatin1();
		keybd_event(vk, 0, 0, 0);
		keybd_event(vk, 0, KEYEVENTF_KEYUP, 0);
	}
}

void Config::loadConfig() {
	QFile file(m_fileName);
	if (!file.open(QIODevice::ReadOnly)) {
		qDebug() << "Using default settings.";
		return;
	}
	m_data = QJsonDocument::fromJson(file.readAll()).object();
	file.close();
	emit hotkeyChanged();
	emit simulateKeyChanged();
	emit intervalChanged();
}

void Config::saveConfig() {
	QFileInfo fileInfo(m_fileName);
	if (!fileInfo.dir().exists())
		fileInfo.dir().mkpath(".");
	QFile file(m_fileName);
	if (file.open(QIODevice::WriteOnly)) {
		file.write(QJsonDocument(m_data).toJson());
		file.close();
	}
}

// Getters
QString Config::hotkey() const { return m_data["hotkey"].toString("F1"); }
QString Config::simulateKey() const {
	return m_data["simulateKey"].toString("LeftClick");
}
int Config::interval() const { return m_data["interval"].toInt(100); }

// Setters (带自动保存)
void Config::setHotkey(const QString &v) {
	if (hotkey() != v) {
		m_data["hotkey"] = v;
		emit hotkeyChanged();
		saveConfig();
	}
}
void Config::setSimulateKey(const QString &v) {
	if (simulateKey() != v) {
		m_data["simulateKey"] = v;
		emit simulateKeyChanged();
		saveConfig();
	}
}
void Config::setInterval(int v) {
	if (interval() != v) {
		m_data["interval"] = v;
		emit intervalChanged();
		saveConfig();
	}
}
void Config::setIsRecording(bool v) {
	if (m_isRecording != v) {
		m_isRecording = v;
		emit isRecordingChanged();
	}
}