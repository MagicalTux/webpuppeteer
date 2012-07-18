#include <QObject>
#include <QScriptEngine>

class WebPuppeteerSys;

class WebPuppeteer: public QObject {
	Q_OBJECT;
public:
	WebPuppeteer(QString file);
	QScriptEngine &engine();

public slots:
	void start();

private:
	QString file;
	int exit_code;

	WebPuppeteerSys *sys;
	QScriptEngine e;
};

