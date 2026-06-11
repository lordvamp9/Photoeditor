#pragma once

#include "core/document.hpp"
#include "filters/filter_base.hpp"

#include <opencv2/core.hpp>

#include <QDialog>
#include <QTimer>

#include <functional>
#include <vector>

namespace PhotoStudio::UI {

// Generic parameter dialog with live (debounced) preview on the active layer.
// On reject the original layer pixels are restored.
class FilterDialog : public QDialog {
    Q_OBJECT

public:
    FilterDialog(const Filters::FilterBase& filter, Core::Document& doc,
                 std::function<void()> repaint, QWidget* parent = nullptr);

protected:
    void done(int result) override;

private:
    void schedulePreview();
    void applyPreview();
    void restoreOriginal();
    Filters::ParamMap currentParams() const;

    const Filters::FilterBase& m_filter;
    Core::Document& m_doc;
    std::function<void()> m_repaint;

    std::vector<Filters::FilterParam> m_specs;
    std::vector<f32> m_values;
    cv::Mat m_backup;
    QTimer m_previewTimer;
    bool m_applied = false;
};

} // namespace PhotoStudio::UI
