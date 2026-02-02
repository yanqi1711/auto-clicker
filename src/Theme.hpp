#ifndef THEME_H
#define THEME_H

#include <QObject>

class Theme : public QObject {
	Q_OBJECT
	Q_PROPERTY(bool isDarkMode READ isDarkMode NOTIFY darkModeChanged)

    public:
	explicit Theme(QObject *parent = nullptr);
	bool isDarkMode() const;

    signals:
	void darkModeChanged();

    private:
	void onColorSchemeChanged();
};

#endif // THEME_H
