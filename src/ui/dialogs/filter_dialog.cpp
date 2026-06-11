#include "ui/dialogs/filter_dialog.hpp"

#include "filters/filter_registry.hpp"

#include <QDialogButtonBox>
#include <QLabel>
#include <QSlider>
#include <QVBoxLayout>

#include <cmath>

namespace PhotoStudio::UI {

FilterDialog::FilterDialog(const Filters::FilterBase& filter, Core::Document& doc,
                           std::function<void()> repaint, QWidget* parent)
    : QDialog(parent), m_filter(filter), m_doc(doc), m_repaint(std::move(repaint))
{
    setWindowTitle(QString::fromStdString(filter.name()));
    setMinimumWidth(360);

    if (auto layer = m_doc.activeLayer())
        m_backup = layer->pixels().clone();

    m_specs = filter.parameters();
    m_values.reserve(m_specs.size());
    for (const auto& spec : m_specs)
        m_values.push_back(spec.defaultValue);

    auto* layout = new QVBoxLayout(this);
    layout->setSpacing(8);

    for (size_t i = 0; i < m_specs.size(); ++i) {
        const auto& spec = m_specs[i];
        auto* label = new QLabel(QString::fromStdString(spec.label));
        label->setProperty("level", "2");
        layout->addWidget(label);

        auto* slider = new QSlider(Qt::Horizontal);
        const f32 scale = spec.step > 0 ? 1.0f / spec.step : 1.0f;
        slider->setRange(static_cast<int>(std::lround(spec.minValue * scale)),
                         static_cast<int>(std::lround(spec.maxValue * scale)));
        slider->setValue(static_cast<int>(std::lround(spec.defaultValue * scale)));
        connect(slider, &QSlider::valueChanged, this, [this, i, scale](int value) {
            m_values[i] = value / scale;
            schedulePreview();
        });
        layout->addWidget(slider);
    }

    if (m_specs.empty()) {
        auto* hint = new QLabel(tr("This filter has no parameters."));
        hint->setProperty("level", "3");
        layout->addWidget(hint);
    }

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);

    m_previewTimer.setSingleShot(true);
    m_previewTimer.setInterval(150);
    connect(&m_previewTimer, &QTimer::timeout, this, &FilterDialog::applyPreview);

    // Initial preview with default parameters.
    schedulePreview();
}

Filters::ParamMap FilterDialog::currentParams() const
{
    Filters::ParamMap params;
    for (size_t i = 0; i < m_specs.size(); ++i)
        params[m_specs[i].key] = m_values[i];
    return params;
}

void FilterDialog::schedulePreview()
{
    m_previewTimer.start();
}

void FilterDialog::applyPreview()
{
    auto layer = m_doc.activeLayer();
    if (!layer || m_backup.empty())
        return;
    layer->pixels() = m_backup.clone();
    m_doc.markDirty();
    Filters::applyFilterToActiveLayer(m_doc, m_filter, currentParams());
    m_applied = true;
    if (m_repaint)
        m_repaint();
}

void FilterDialog::restoreOriginal()
{
    auto layer = m_doc.activeLayer();
    if (!layer || m_backup.empty())
        return;
    layer->pixels() = m_backup.clone();
    m_doc.markDirty();
    if (m_repaint)
        m_repaint();
}

void FilterDialog::done(int result)
{
    m_previewTimer.stop();
    if (result == QDialog::Accepted) {
        if (!m_applied)
            applyPreview();
    } else {
        restoreOriginal();
    }
    QDialog::done(result);
}

} // namespace PhotoStudio::UI
