#pragma once

#include <QString>

#include "task/Task.h"

namespace Ralph {
namespace ClientLib {
class Project;

class ProjectGenerator
{
public:
	explicit ProjectGenerator();

	QString name() const { return m_name; }
	void setName(const QString &name) { m_name = name; }
	QString vcs() const { return m_vcs; }
	void setVCS(const QString &vcs) { m_vcs = vcs; }
	QString buildSystem() const { return m_bs; }
	void setBuildSystem(const QString &bs) { m_bs = bs; }
	QString directory() const { return m_directory; }
	void setDirectory(const QString &directory) { m_directory = directory; }

	Future<Project *> generate() const;

private:
	QString m_name;
	QString m_vcs;
	QString m_bs;
	QString m_directory;

	void copyTemplate(const QString &name, const QString &destination) const;
};
}
}
