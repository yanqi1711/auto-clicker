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

	Q_INVOKABLE void startRecording(QString target);
	Q_INVOKABLE void stopAutoClick();

	void setHotkey(const QString &name, int vk);
	void setHotkey(const QString &v) {}
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
	void performAction();
	void saveConfig();

private:
	static Config *instance;
	static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam,
	                                             LPARAM lParam);
	static LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam,
	                                          LPARAM lParam);

	void startHook();
	void toggleAutoClick();
	void markDirty();

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