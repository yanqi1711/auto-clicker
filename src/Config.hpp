#ifndef CONFIG_H
#define CONFIG_H

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QTimer>
#include <windows.h>

class Config : public QObject {
	Q_OBJECT
	// 属性绑定：QML 直接访问
	Q_PROPERTY(QString hotkey READ hotkey WRITE setHotkey NOTIFY hotkeyChanged)
	Q_PROPERTY(QString simulateKey READ simulateKey WRITE setSimulateKey NOTIFY
	               simulateKeyChanged)
	Q_PROPERTY(
	    int interval READ interval WRITE setInterval NOTIFY intervalChanged)
	Q_PROPERTY(bool isRecording READ isRecording WRITE setIsRecording NOTIFY
	               isRecordingChanged)
	Q_PROPERTY(bool isRunning READ isRunning NOTIFY isRunningChanged)

public:
	explicit Config(QObject *parent = nullptr);
	~Config();

	static Config *getInstance();

	// 基础 Getter
	bool isRunning() const {
		return m_isRunning;
	}
	bool isRecording() const {
		return m_isRecording;
	}
	QString hotkey() const {
		return m_data["hotkeyName"].toString("F1");
	}
	QString simulateKey() const {
		return m_data["simulateKeyName"].toString("LeftClick");
	}
	int interval() const {
		return m_data["interval"].toInt(100);
	}

	// QML 调用的方法
	Q_INVOKABLE void startRecording(QString target);
	Q_INVOKABLE void stopAutoClick();

	// 属性 Setter
	void setHotkey(const QString &name, int vk); // 内部使用，同时存名称和码
	void setHotkey(const QString &v) { /* 兼容旧接口 */ }
	void setSimulateKey(const QString &name, int vk);
	void setSimulateKey(const QString &v) {}
	void setInterval(int v);
	void setIsRecording(bool v);

signals:
	void hotkeyChanged();
	void simulateKeyChanged();
	void intervalChanged();
	void isRecordingChanged();
	void isRunningChanged();

private slots:
	void performAction(); // 连点执行
	void saveConfig();    // 实际写磁盘

private:
	static Config *instance;
	static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam,
	                                             LPARAM lParam);
	static LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam,
	                                          LPARAM lParam);

	void startHook();
	void toggleAutoClick();
	void markDirty(); // 触发延迟保存

	QJsonObject m_data;
	const QString m_fileName = "config/config.json";
	bool m_isRecording = false;
	bool m_isRunning = false;
	QString m_recordingTarget;

	HHOOK m_kbdHook = nullptr;
	HHOOK m_mouseHook = nullptr;
	QTimer *m_clickTimer;
	QTimer *m_saveTimer;
};

#endif // CONFIG_H