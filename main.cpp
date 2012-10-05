#include <QApplication>
#include <QTimer>
#include <QStringList>
#include <QWebSettings>
#include <QSslSocket>
#include "WebPuppeteer.hpp"

int main(int argc, char *argv[]) {
	QApplication app(argc, argv);

	QStringList args = app.arguments();
	QString file;

	if (!QSslSocket::supportsSsl()) {
		qDebug("WARNING: SSL not supported");
	}

	if (args.size() > 1) {
		file = args.at(1);
	} else {
		file = "test.js";
	}

	QWebSettings::globalSettings()->setAttribute(QWebSettings::JavascriptCanOpenWindows, true);
	QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);

	WebPuppeteer wp(file);
	
	// trick so wp.start() can actually call stuff like QCoreApplication::exit(x)
	QTimer::singleShot(0, &wp, SLOT(start()));

	app.setQuitOnLastWindowClosed(false);

	qDebug("Running on Qt %s", qVersion());

	return app.exec();
}

