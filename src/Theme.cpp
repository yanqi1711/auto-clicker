#include "Theme.hpp"
#include <QGuiApplication>
#include <QStyleHints>

Theme::Theme(QObject *parent)
	: QObject(parent)
{
	connect(qGuiApp->styleHints(), &QStyleHints::colorSchemeChanged, this,
		&Theme::onColorSchemeChanged);
}

bool Theme::isDarkMode() const
{
	return qGuiApp->styleHints()->colorScheme() == Qt::ColorScheme::Dark;
}

void Theme::onColorSchemeChanged()
{
	emit darkModeChanged();
}
