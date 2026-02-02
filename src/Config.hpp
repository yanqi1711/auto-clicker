#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QTimer>
#include <windows.h>

class Config : public QObject {
	Q_OBJECT
	Q_PROPERTY(QString hotkey READ hotkey WRITE setHotkey NOTIFY hotkeyChanged)
	Q_PROPERTY(QString simulateKey READ simulateKey WRITE setSimulateKey NOTIFY
	               simulateKeyChanged)
	Q_PROPERTY(
	    int interval READ interval WRITE setInterval NOTIFY intervalChanged)
	// 新增：录制状态属性
	Q_PROPERTY(bool isRecording READ isRecording WRITE setIsRecording NOTIFY
	               isRecordingChanged)
	Q_PROPERTY(bool isRunning READ isRunning NOTIFY isRunningChanged)

public:
	explicit Config(QObject *parent = nullptr);
	~Config();

	bool isRunning() const { return m_isRunning; }

	Q_INVOKABLE void loadConfig();
	Q_INVOKABLE void saveConfig();
	HHOOK m_mouseHook; // 新增鼠标钩子句柄
	static LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam,
	                                          LPARAM lParam);

	// 提供给全局钩子调用的静态函数
	static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam,
	                                             LPARAM lParam);
	static Config *instance;

	QString hotkey() const;
	QString simulateKey() const;
	// 在 private 增加一个成员
	QString m_recordingTarget; // "hotkey" 或 "simulateKey"

	// 在 public 增加一个方法给 QML 调用
	Q_INVOKABLE void startRecording(QString target) {
		m_recordingTarget = target;
		setIsRecording(true);
	}
	int interval() const;
	bool isRecording() const { return m_isRecording; }

	void setHotkey(const QString &v);
	void setSimulateKey(const QString &v);
	void setInterval(int v);
	void setIsRecording(bool v);

signals:
	void hotkeyChanged();
	void simulateKeyChanged();
	void intervalChanged();
	void isRecordingChanged();
	void isRunningChanged();

private:
	QJsonObject m_data;
	bool m_isRecording = false;
	const QString m_fileName = "config/config.json";
	bool m_isRunning = false;
	QTimer *m_clickTimer;
	HHOOK m_hook;

	void toggleAutoClick(); // 切换运行状态
	void performAction();   // 执行模拟点击
	void startHook();       // 启动钩子
};

#endif