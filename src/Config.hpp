#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <QJsonObject>
#include <QObject>
#include <QString>

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

public:
	explicit Config(QObject *parent = nullptr);

	Q_INVOKABLE void loadConfig();
	Q_INVOKABLE void saveConfig();

	QString hotkey() const;
	QString simulateKey() const;
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

private:
	QJsonObject m_data;
	bool m_isRecording = false;
	const QString m_fileName = "config/config.json";
};

#endif