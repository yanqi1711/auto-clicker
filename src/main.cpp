#include "Config.hpp"
#include "Theme.hpp"
#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>

int main(int argc, char *argv[]) {
	QQuickStyle::setStyle("Material");

	QGuiApplication app(argc, argv);
	QQmlApplicationEngine engine;

	Theme theme;
	Config config;

	const QUrl url(QStringLiteral("qrc:/res/main.qml"));

	app.setWindowIcon(QIcon(":/icon"));

	engine.rootContext()->setContextProperty("theme", &theme);
	engine.rootContext()->setContextProperty("cfg", &config);

	QObject::connect(
	    &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
	    []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);

	engine.load(url);

	if (engine.rootObjects().isEmpty())
		return -1;

	return app.exec();
}