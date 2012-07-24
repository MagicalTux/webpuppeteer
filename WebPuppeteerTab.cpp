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
#include <QTimer>
#include <QTemporaryFile>

WebPuppeteerTab::WebPuppeteerTab(WebPuppeteer *_parent): QWebPage(_parent) {
	parent = _parent;

	// define standard values
	setViewportSize(QSize(1024,768));
	setForwardUnsupportedContent(true);

	connect(this, SIGNAL(unsupportedContent(QNetworkReply*)), this, SLOT(downloadFile(QNetworkReply*)));
	connect(networkAccessManager(), SIGNAL(sslErrors(QNetworkReply*,const QList<QSslError>&)), this, SLOT(handleSslErrors(QNetworkReply*,const QList<QSslError>&)));
}

QWebPage *WebPuppeteerTab::createWindow(WebWindowType) {
	// we don't care about the type, since modal has no meaning here
	WebPuppeteerTab *tab = new WebPuppeteerTab(parent);
	queue.append(tab);
	return tab;
}

QScriptValue WebPuppeteerTab::getNewWindow() {
	if (queue.size() == 0) return parent->engine().nullValue();
	return parent->engine().newQObject(queue.takeFirst(), QScriptEngine::ScriptOwnership);
}

void WebPuppeteerTab::trustCertificate(const QString &hash) {
	trusted_certificates.insert(QByteArray::fromHex(hash.toLatin1()));
}

void WebPuppeteerTab::handleSslErrors(QNetworkReply *reply,const QList<QSslError>&list) {
	bool ignore_ok = true;

	for(int i = 0; i < list.size(); i++) {
		bool this_ignore_ok = false;
		switch(list.at(i).error()) {
			case QSslError::UnableToGetLocalIssuerCertificate:
			case QSslError::CertificateUntrusted:
			case QSslError::UnableToVerifyFirstCertificate:
			{
				QByteArray hash = list.at(i).certificate().digest(QCryptographicHash::Sha1);
				if (trusted_certificates.contains(hash)) {
					this_ignore_ok = true;
				} else {
					qDebug("The following error could be ignored by calling tab.trustCertificate(\"%s\")", hash.toHex().data());
				}
			}
			default: break;
		}

		if (!this_ignore_ok) {
			qDebug("SSL Error: %d %s", list.at(i).error(), qPrintable(list.at(i).errorString()));
			ignore_ok = false;
		}
	}

	if (ignore_ok)
		reply->ignoreSslErrors();
}

bool WebPuppeteerTab::shouldInterruptJavaScript() {
	return true;
}

void WebPuppeteerTab::javaScriptAlert(QWebFrame*, const QString &msg) {
	qDebug("Got javascript alert: %s", qPrintable(msg));
}

bool WebPuppeteerTab::supportsExtension(Extension e) {
	switch(e) {
		case QWebPage::ChooseMultipleFilesExtension: return true;
		case QWebPage::ErrorPageExtension: return true;
		default: return false;
	}
}

bool WebPuppeteerTab::extension(Extension e, ExtensionOption *option, ExtensionReturn *output) {
	switch(e) {
		case QWebPage::ChooseMultipleFilesExtension:
		{
			if (output == NULL) return false;
			//ChooseMultipleFilesExtensionReturn *ret = (ChooseMultipleFilesExtensionReturn*)output;

			// TODO
			
			return true;
		}
		case QWebPage::ErrorPageExtension:
		{
			ErrorPageExtensionOption *opt = (ErrorPageExtensionOption*)option;
			ErrorPageExtensionReturn *ret = (ErrorPageExtensionReturn*)output;

			qDebug("HTTP error %d: %s", opt->error, qPrintable(opt->errorString));
			if (ret != 0) {
				ret->baseUrl = QUrl("http://localhost/");
				ret->contentType = "text/html";
				ret->encoding = "UTF-8";
				ret->content = "<h1>Load error</h1><p>Could not load page :(</p>";
			}
			return true;
		}
		default:
			return false;
	}
}

void WebPuppeteerTab::downloadFile(QNetworkReply*reply) {
	// Won't be finished at this point, need to connect() to reply signals to detect end of download
	// Just in case, check if reply->isFinished()
	if (reply->isFinished()) {
		downloadFileFinished(reply);
	}
	connect(reply, SIGNAL(finished()), this, SLOT(downloadFileFinished()));
}

void WebPuppeteerTab::downloadFileFinished(QNetworkReply*reply) {
	if (reply == NULL)
		reply = qobject_cast<QNetworkReply*>(sender());

	if (reply->error() != QNetworkReply::NoError) {
		qDebug("error downloading file: %d", reply->error());
		return;
	}
	QString filename = reply->request().url().path();
	int idx = filename.lastIndexOf('/');
	if (idx != -1) filename = filename.mid(idx+1);

	QScriptValue arr = parent->engine().newObject();
	arr.setProperty("filename", filename);
	arr.setProperty("filedata", QString::fromLatin1(reply->readAll().toBase64()));

	file_queue.append(arr);

	qDebug("File download success: %s", qPrintable(filename));
}

QScriptValue WebPuppeteerTab::getDownloadedFile() {
	if (file_queue.isEmpty()) return parent->engine().nullValue();
	return file_queue.takeFirst();
}

WebPuppeteer *WebPuppeteerTab::getParent() {
	return parent;
}

bool WebPuppeteerTab::load(const QString &url, int timeout) {
	mainFrame()->load(QUrl(url));
	return wait(timeout);
}

bool WebPuppeteerTab::browse(const QString &url, int timeout) {
	mainFrame()->load(mainFrame()->url().resolved(QUrl(url)));
	return wait(timeout);
}

void WebPuppeteerTab::back() {
	triggerAction(QWebPage::Back);
}

void WebPuppeteerTab::reload(bool force_no_cache) {
	if (force_no_cache) {
		triggerAction(QWebPage::ReloadAndBypassCache);
	} else {
		triggerAction(QWebPage::Reload);
	}
}

void WebPuppeteerTab::setReturnBool(bool r) {
	return_bool = r;
}

bool WebPuppeteerTab::screenshot(const QString &filename) {
	// disable scrollbars, not as anyone is going to use them anyway
	mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
	mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);

	QImage img(viewportSize(), QImage::Format_RGB32);
	QPainter paint(&img);
	mainFrame()->render(&paint);
	paint.end();

	mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAsNeeded);
	mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAsNeeded);

	return img.save(filename);
}

bool WebPuppeteerTab::fullshot(const QString &filename) {
	// disable scrollbars, not as anyone is going to use them anyway
	mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
	mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);

	QImage img(mainFrame()->contentsSize(), QImage::Format_RGB32);
	QPainter paint(&img);
	mainFrame()->render(&paint);
	paint.end();

	mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAsNeeded);
	mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAsNeeded);

	return img.save(filename);
}

bool WebPuppeteerTab::print(const QString &filename) {
	QPrinter print;
	print.setOutputFileName(filename);
	print.setOutputFormat(QPrinter::PdfFormat);
	print.setPaperSize(QPrinter::A4);

//	print.setPageMargins(0, 0, 0, 0, QPrinter::Inch);
	// we know our page is 72dpi, how many dpi do we need on the printer to fit it ?
//	int dpi = (mainFrame()->contentsSize().width() * print.paperSize(QPrinter::Inch).width() / 72.0) * 1.06;
//	print.setResolution(dpi);
//	QPainter print_p(&print);
//	QSize size = mainFrame()->contentsSize();
//	mainFrame()->render(&print_p, QWebFrame::ContentsLayer, QRegion(0, 0, size.width(), size.height()));
//	print_p.end();
	mainFrame()->print(&print);
	return true;
}

QString WebPuppeteerTab::printBase64() {
	QTemporaryFile t;
	if (!t.open()) return QString();
	if (!print(t.fileName())) return QString();
	QByteArray data = t.readAll();
	t.remove();
	return QString::fromLatin1(data.toBase64());
}

QScriptValue WebPuppeteerTab::eval(const QString &js) {
	return parent->engine().newVariant(mainFrame()->evaluateJavaScript(js));
}

QScriptValue WebPuppeteerTab::document() {
	return parent->engine().newQObject(new WebPuppeteerWebElement(this, mainFrame()->documentElement()), QScriptEngine::ScriptOwnership);
}

QString WebPuppeteerTab::treeDump() {
	return mainFrame()->renderTreeDump();
}

void WebPuppeteerTab::interact() {
	QEventLoop e;

	QWebView *v = new QWebView();
	v->setAttribute(Qt::WA_DeleteOnClose, true);
	connect(v, SIGNAL(destroyed(QObject*)), &e, SLOT(quit()));
	v->setPage(this);
	v->show();
	e.exec();
}

QScriptValue WebPuppeteerTab::get(const QString &url) {
	QNetworkRequest req(url);
	QNetworkReply *rep = networkAccessManager()->get(req);
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

bool WebPuppeteerTab::wait(int timeout) {
	QEventLoop e;

	connect(this, SIGNAL(loadFinished(bool)), &e, SLOT(quit()));
	connect(this, SIGNAL(loadFinished(bool)), this, SLOT(setReturnBool(bool)));
	if (timeout > 0)
		QTimer::singleShot(timeout*1000, &e, SLOT(quit()));
	return_bool = false;

	e.exec();

	return return_bool;
}

void WebPuppeteerTab::type(const QString &text) {
	QKeyEvent ev(QEvent::KeyPress, 0, Qt::NoModifier, text);
	event(&ev);
}

void WebPuppeteerTab::typeEnter() {
	QKeyEvent ev(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
	event(&ev);
}

void WebPuppeteerTab::typeTab() {
	QKeyEvent ev(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
	event(&ev);
}

QString WebPuppeteerTab::getHtml() {
	return mainFrame()->toHtml();
}

