#include "WebPuppeteerTab.hpp"
#include "WebPuppeteerWebElement.hpp"
#include "WebPuppeteer.hpp"
#include <QEventLoop>
#include <QWebFrame>
#include <QPainter>
#include <QPrinter>
#include <QWebView>
#include <QNetworkReply>
#include <QEvent>
#include <QKeyEvent>

WebPuppeteerTab::WebPuppeteerTab(WebPuppeteer *_parent): QObject(_parent) {
	parent = _parent;
	page = new QWebPage(this);

	// define standard values
	page->setViewportSize(QSize(1024,768));
	page->setForwardUnsupportedContent(true);

	// disable scrollbars, not as anyone is going to use them anyway
	page->mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
	page->mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);

	connect(page, SIGNAL(unsupportedContent(QNetworkReply*)), this, SLOT(downloadFile(QNetworkReply*)));
}

void WebPuppeteerTab::downloadFile(QNetworkReply*reply) {
	// test stuff
	qDebug("finished: %s", reply->isFinished() ? "yes":"no");
}

WebPuppeteer *WebPuppeteerTab::getParent() {
	return parent;
}

bool WebPuppeteerTab::browse(const QString &url) {
	page->mainFrame()->load(QUrl(url));
	return wait();
}

void WebPuppeteerTab::setReturnBool(bool r) {
	return_bool = r;
}

bool WebPuppeteerTab::screenshot(const QString &filename) {
	QImage img(page->viewportSize(), QImage::Format_RGB32);
	QPainter paint(&img);
	page->mainFrame()->render(&paint);
	paint.end();
	return img.save(filename);
}

bool WebPuppeteerTab::fullshot(const QString &filename) {
	QImage img(page->mainFrame()->contentsSize(), QImage::Format_RGB32);
	QPainter paint(&img);
	page->mainFrame()->render(&paint);
	paint.end();
	return img.save(filename);
}

bool WebPuppeteerTab::print(const QString &filename) {
	QPrinter print;
	print.setOutputFileName(filename);
	print.setOutputFormat(QPrinter::PdfFormat);
	print.setPaperSize(QPrinter::A4);

//	print.setPageMargins(0, 0, 0, 0, QPrinter::Inch);
	// we know our page is 72dpi, how many dpi do we need on the printer to fit it ?
//	int dpi = (page->mainFrame()->contentsSize().width() * print.paperSize(QPrinter::Inch).width() / 72.0) * 1.06;
//	print.setResolution(dpi);
//	QPainter print_p(&print);
//	QSize size = page->mainFrame()->contentsSize();
//	page->mainFrame()->render(&print_p, QWebFrame::ContentsLayer, QRegion(0, 0, size.width(), size.height()));
//	print_p.end();
	page->mainFrame()->print(&print);
	return true;
}

QScriptValue WebPuppeteerTab::eval(const QString &js) {
	return parent->engine().newVariant(page->mainFrame()->evaluateJavaScript(js));
}

QScriptValue WebPuppeteerTab::document() {
	return parent->engine().newQObject(new WebPuppeteerWebElement(this, page->mainFrame()->documentElement()));
}

QString WebPuppeteerTab::treeDump() {
	return page->mainFrame()->renderTreeDump();
}

void WebPuppeteerTab::interact() {
	QEventLoop e;

	QWebView *v = new QWebView();
	v->setAttribute(Qt::WA_DeleteOnClose, true);
	connect(v, SIGNAL(destroyed(QObject*)), &e, SLOT(quit()));
	v->setPage(page);
	v->show();
	e.exec();
}

QScriptValue WebPuppeteerTab::get(const QString &url) {
	QNetworkRequest req(url);
	QNetworkReply *rep = page->networkAccessManager()->get(req);
	QEventLoop e;

	connect(rep, SIGNAL(finished()), &e, SLOT(quit()));
	e.exec();

	if (rep->error() != QNetworkReply::NoError) {
		qDebug("GET error: %s", qPrintable(rep->errorString()));
		rep->deleteLater();
		return parent->engine().currentContext()->throwError(QScriptContext::UnknownError, rep->errorString());
	}

	return parent->engine().newVariant((rep->readAll()));
}

bool WebPuppeteerTab::wait() {
	QEventLoop e;

	connect(page, SIGNAL(loadFinished(bool)), &e, SLOT(quit()));
	connect(page, SIGNAL(loadFinished(bool)), this, SLOT(setReturnBool(bool)));
	e.exec();

	return return_bool;
}

void WebPuppeteerTab::type(const QString &text) {
	QKeyEvent ev(QEvent::KeyPress, 0, Qt::NoModifier, text);
	page->event(&ev);
}

void WebPuppeteerTab::typeEnter() {
	QKeyEvent ev(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
	page->event(&ev);
}

bool WebPuppeteerTab::click(const QString &text) {
	// search a button/link/input[type=submit] element with this text
	QWebElement el = page->mainFrame()->documentElement();

	while(true) {
		bool check = false;
		QString str;

		if ((el.tagName() == "A") && (el.hasAttribute("href"))) {
			str = el.toPlainText();
			check = true;
		} else if (el.tagName() == "BUTTON") {
			str = el.toPlainText();
			check = true;
		} else if ((el.tagName() == "INPUT") && (el.attribute("type").toLower() == "submit")) {
			str = el.attribute("value");
			check = true;
		}

		if ((check) && (str.contains(text, Qt::CaseInsensitive))) {
			return el.evaluateJavaScript("(function(obj) { var e = document.createEvent('MouseEvents'); e.initEvent('click',true,false); return obj.dispatchEvent(e); })(this)").toBool();
		}

		QWebElement tmp = el.firstChild();
		if (!tmp.isNull()) {
			el = tmp;
			continue;
		}
		tmp = el.nextSibling();
		if (!tmp.isNull()) {
			el = tmp;
			continue;
		}

		bool exit = false;
		while(true) {
			tmp = el.parent();
			if (tmp.isNull()) {
				exit = true;
				break;
			}
			el = tmp;
			tmp = el.nextSibling();
			if (!tmp.isNull()) {
				el = tmp;
				break;
			}
		}
		if (exit) break;
	}

	return false;
}

