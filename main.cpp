#include <QApplication>
#include <QTimer>
#include <QStringList>
#include "WebPuppeteer.hpp"

int main(int argc, char *argv[]) {
	QApplication app(argc, argv);

	QStringList args = app.arguments();
	QString file;

	if (args.size() > 1) {
		file = args.at(1);
	} else {
		file = "test.js";
	}

	WebPuppeteer wp(file);
	
	// trick so wp.start() can actually call stuff like QCoreApplication::exit(x)
	QTimer::singleShot(0, &wp, SLOT(start()));

	return app.exec();
}

