#include "Theme.hpp"
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QIcon>

int main(int argc, char *argv[])
{
	QQuickStyle::setStyle("Material");

	QGuiApplication app(argc, argv);

	app.setWindowIcon(QIcon(":/icon"));

	Theme theme;
	QQmlApplicationEngine engine;

	engine.rootContext()->setContextProperty("theme", &theme);

	const QUrl url(QStringLiteral("qrc:/res/main.qml"));

	QObject::connect(
		&engine, &QQmlApplicationEngine::objectCreationFailed, &app,
		[]() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);

	engine.load(url);

	if (engine.rootObjects().isEmpty()) {
		return -1;
	}

	return app.exec();
}
