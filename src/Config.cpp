#include "Config.hpp"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>

Config::Config(QObject *parent) : QObject(parent) { loadConfig(); }

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