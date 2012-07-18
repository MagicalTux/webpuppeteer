#include <QObject>

class WebPuppeteerConsole;

class WebPuppeteer: public QObject {
	Q_OBJECT;
public:
	WebPuppeteer(QString file);

public slots:
	void start();

private:
	QString file;
	int exit_code;

	WebPuppeteerConsole *console;
};

