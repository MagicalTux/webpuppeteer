#include <QApplication>
#include <QTimer>
#include "WebPuppeteer.hpp"

int main(int argc, char *argv[]) {
	QApplication app(argc, argv);

	WebPuppeteer wp("test.js");
	
	// trick so wp.start() can actually call stuff like QCoreApplication::exit(x)
	QTimer::singleShot(0, &wp, SLOT(start()));

	return app.exec();
}

