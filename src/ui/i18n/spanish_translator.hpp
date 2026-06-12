#pragma once

#include <QHash>
#include <QTranslator>

namespace PhotoStudio::UI {

// Map-based Spanish translation. Source strings are the English literals used
// across the UI (and the plain-string names coming from the Qt-free core),
// looked up regardless of context so core strings translate too.
class SpanishTranslator : public QTranslator {
    Q_OBJECT

public:
    explicit SpanishTranslator(QObject* parent = nullptr);

    QString translate(const char* context, const char* sourceText, const char* disambiguation,
                      int n) const override;
    bool isEmpty() const override { return false; }

private:
    QHash<QString, QString> m_map;
};

} // namespace PhotoStudio::UI
