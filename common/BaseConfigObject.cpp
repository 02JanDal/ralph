#include "BaseConfigObject.h"

#include <QTimer>
#include <QFile>
#include <QCoreApplication>
#include <QDebug>

#include "Exception.h"
#include "FileSystem.h"

BaseConfigObject::BaseConfigObject(const QString &filename)
	: m_filename(filename)
{
	m_saveTimer = new QTimer;
	m_saveTimer->setSingleShot(true);
	// cppcheck-suppress pureVirtualCall
	QObject::connect(m_saveTimer, &QTimer::timeout, [this]() { saveNow(); });
	setSaveTimeout(250);

	QTimer::singleShot(0, [this]() { loadNow(); });

	// cppcheck-suppress pureVirtualCall
	m_appQuitConnection = QObject::connect(qApp, &QCoreApplication::aboutToQuit, [this]() { saveNow(); });
}
BaseConfigObject::~BaseConfigObject()
{
	delete m_saveTimer;
	QObject::disconnect(m_appQuitConnection);
}

void BaseConfigObject::setSaveTimeout(int msec)
{
	m_saveTimer->setInterval(msec);
}

void BaseConfigObject::scheduleSave()
{
	m_saveTimer->stop();
	m_saveTimer->start();
}
void BaseConfigObject::saveNow()
{
	if (m_saveTimer->isActive())
	{
		m_saveTimer->stop();
	}
	if (m_disableSaving)
	{
		return;
	}

	try
	{
		FS::write(m_filename, doSave());
	}
	catch (Exception & e)
	{
		qCritical() << e.cause();
	}
}
void BaseConfigObject::loadNow()
{
	if (m_saveTimer->isActive())
	{
		saveNow();
	}

	try
	{
		doLoad(FS::read(m_filename));
	}
	catch (Exception & e)
	{
		qWarning() << "Error loading" << m_filename << ":" << e.cause();
	}
}
