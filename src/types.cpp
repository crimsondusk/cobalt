#include <cassert>
#include <QObject>
#include <QStringList>
#include <QTextStream>
#include <QFile>
#include <QPoint>
#include "types.h"

// =============================================================================
// -----------------------------------------------------------------------------
str DoFormat (const QList<StringFormatArg>& args) {
	assert (args.size() >= 1);
	str text = args[0].value();
	
	for (uchar i = 1; i < args.size(); ++i)
		text = text.arg (args[i].value());
	
	return text;
}

void doPrint (FILE* f, const QList<StringFormatArg>& args) {
	str out = DoFormat (args);
	fprintf (f, "%s", out.toStdString().c_str());
}

void doDebug (const char* func, const QList<StringFormatArg>& args) {
	fprintf (stderr, "%s: %s", func, DoFormat (args).toStdString().c_str());
}

// =============================================================================
// -----------------------------------------------------------------------------
StringFormatArg::StringFormatArg (const str& v) {
	m_val = v;
}

StringFormatArg::StringFormatArg (const char& v) {
	m_val = v;
}

StringFormatArg::StringFormatArg (const uchar& v) {
	m_val = v;
}

StringFormatArg::StringFormatArg (const qchar& v) {
	m_val = v;
}

StringFormatArg::StringFormatArg (const float& v) {
	m_val.setNum (v);
}

StringFormatArg::StringFormatArg (const double& v) {
	m_val.setNum (v);
}

StringFormatArg::StringFormatArg (const char* v) {
	m_val = v;
}

StringFormatArg::StringFormatArg (const void* v) {
	m_val.sprintf ("%p", v);
}

StringFormatArg::StringFormatArg (const QVariant& v) {
	m_val = v.toString();
}

StringFormatArg::StringFormatArg (const QPoint& v) {
	m_val = fmt ("(%1, %2)", v.x(), v.y());
}

StringFormatArg::StringFormatArg (const QPointF& v) {
	m_val = fmt ("(%1, %2)", v.x(), v.y());
}