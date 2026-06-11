#include "ui/panels/layers_panel.hpp"

#include "ui/image_convert.hpp"

#include <opencv2/imgproc.hpp>

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include <algorithm>

namespace PhotoStudio::UI {

namespace {

QIcon layerThumbnail(const Core::Layer& layer)
{
    cv::Mat small;
    const f64 scale = 40.0 / std::max(layer.width(), layer.height());
    cv::resize(layer.pixels(), small, cv::Size(), scale, scale, cv::INTER_AREA);
    return QIcon(QPixmap::fromImage(matToQImage(small)));
}

QPushButton* smallButton(const QString& text, const QString& tooltip)
{
    auto* btn = new QPushButton(text);
    btn->setToolTip(tooltip);
    btn->setFixedSize(28, 24);
    btn->setProperty("class", "panelAction");
    return btn;
}

} // namespace

LayersPanel::LayersPanel(QWidget* parent) : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(8);

    auto* blendRow = new QHBoxLayout();
    auto* blendLabel = new QLabel(tr("Blend"));
    blendLabel->setProperty("level", "2");
    m_blendMode = new QComboBox();
    for (const auto& name : Core::blendModeNames())
        m_blendMode->addItem(QString::fromStdString(name));
    blendRow->addWidget(blendLabel);
    blendRow->addWidget(m_blendMode, 1);
    layout->addLayout(blendRow);

    auto* opacityRow = new QHBoxLayout();
    auto* opacityLabel = new QLabel(tr("Opacity"));
    opacityLabel->setProperty("level", "2");
    m_opacity = new QSlider(Qt::Horizontal);
    m_opacity->setRange(0, 100);
    m_opacity->setValue(100);
    opacityRow->addWidget(opacityLabel);
    opacityRow->addWidget(m_opacity, 1);
    layout->addLayout(opacityRow);

    m_list = new QListWidget();
    m_list->setIconSize(QSize(40, 40));
    m_list->setSelectionMode(QAbstractItemView::SingleSelection);
    layout->addWidget(m_list, 1);

    auto* buttons = new QHBoxLayout();
    buttons->setSpacing(4);
    auto* add = smallButton("+", tr("New layer"));
    auto* dup = smallButton("=", tr("Duplicate layer"));
    auto* up = smallButton("^", tr("Move layer up"));
    auto* down = smallButton("v", tr("Move layer down"));
    auto* merge = smallButton("M", tr("Merge down"));
    auto* del = smallButton("-", tr("Delete layer"));
    for (auto* b : {add, dup, up, down, merge, del})
        buttons->addWidget(b);
    buttons->addStretch(1);
    layout->addLayout(buttons);

    connect(m_list, &QListWidget::itemChanged, this, &LayersPanel::onItemChanged);
    connect(m_list, &QListWidget::itemSelectionChanged, this, &LayersPanel::onSelectionChanged);
    connect(add, &QPushButton::clicked, this, &LayersPanel::onAddLayer);
    connect(dup, &QPushButton::clicked, this, &LayersPanel::onDuplicateLayer);
    connect(up, &QPushButton::clicked, this, &LayersPanel::onMoveUp);
    connect(down, &QPushButton::clicked, this, &LayersPanel::onMoveDown);
    connect(merge, &QPushButton::clicked, this, &LayersPanel::onMergeDown);
    connect(del, &QPushButton::clicked, this, &LayersPanel::onDeleteLayer);

    connect(m_opacity, &QSlider::valueChanged, this, [this](int value) {
        if (m_updating || !m_doc)
            return;
        if (auto layer = m_doc->activeLayer()) {
            layer->setOpacity(value / 100.0f);
            m_doc->markDirty();
            emit compositeChanged();
        }
    });
    connect(m_opacity, &QSlider::sliderReleased, this,
            [this] { emit documentEdited(tr("Layer Opacity")); });

    connect(m_blendMode, &QComboBox::currentIndexChanged, this, [this](int index) {
        if (m_updating || !m_doc || index < 0)
            return;
        if (auto layer = m_doc->activeLayer()) {
            layer->setBlendMode(static_cast<Core::BlendMode>(index));
            m_doc->markDirty();
            emit compositeChanged();
            emit documentEdited(tr("Blend Mode"));
        }
    });
}

void LayersPanel::setDocument(Core::Document* doc)
{
    m_doc = doc;
    refresh();
}

void LayersPanel::refresh()
{
    m_updating = true;
    m_list->clear();
    if (m_doc) {
        // Top layer first.
        const auto& layers = m_doc->layers();
        for (auto it = layers.rbegin(); it != layers.rend(); ++it) {
            const auto& layer = *it;
            auto* item = new QListWidgetItem(layerThumbnail(*layer),
                                             QString::fromStdString(layer->name()));
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsEditable);
            item->setCheckState(layer->isVisible() ? Qt::Checked : Qt::Unchecked);
            item->setData(Qt::UserRole, layer->id());
            m_list->addItem(item);
            if (layer->id() == m_doc->activeLayerId())
                m_list->setCurrentItem(item);
        }
    }
    m_updating = false;
    syncControlsToActiveLayer();
}

void LayersPanel::syncControlsToActiveLayer()
{
    if (!m_doc)
        return;
    auto layer = m_doc->activeLayer();
    if (!layer)
        return;
    m_updating = true;
    m_opacity->setValue(static_cast<int>(layer->opacity() * 100));
    m_blendMode->setCurrentIndex(static_cast<int>(layer->blendMode()));
    m_updating = false;
}

void LayersPanel::onItemChanged(QListWidgetItem* item)
{
    if (m_updating || !m_doc)
        return;
    const u32 id = item->data(Qt::UserRole).toUInt();
    auto layer = m_doc->layerById(id);
    if (!layer)
        return;

    const bool visible = item->checkState() == Qt::Checked;
    const std::string newName = item->text().toStdString();
    bool edited = false;

    if (layer->isVisible() != visible) {
        layer->setVisible(visible);
        m_doc->markDirty();
        emit compositeChanged();
        edited = true;
    }
    if (!newName.empty() && layer->name() != newName) {
        layer->setName(newName);
        edited = true;
    }
    if (edited)
        emit documentEdited(tr("Layer Properties"));
}

void LayersPanel::onSelectionChanged()
{
    if (m_updating || !m_doc)
        return;
    auto* item = m_list->currentItem();
    if (!item)
        return;
    m_doc->setActiveLayer(item->data(Qt::UserRole).toUInt());
    syncControlsToActiveLayer();
}

void LayersPanel::onAddLayer()
{
    if (!m_doc)
        return;
    auto layer = std::make_shared<Core::Layer>(m_doc->width(), m_doc->height());
    layer->setName("Layer " + std::to_string(m_doc->layers().size() + 1));
    m_doc->addLayer(layer);
    refresh();
    emit compositeChanged();
    emit documentEdited(tr("New Layer"));
}

void LayersPanel::onDeleteLayer()
{
    if (!m_doc || m_doc->layers().size() <= 1)
        return;
    m_doc->removeLayer(m_doc->activeLayerId());
    refresh();
    emit compositeChanged();
    emit documentEdited(tr("Delete Layer"));
}

void LayersPanel::onDuplicateLayer()
{
    if (!m_doc)
        return;
    m_doc->duplicateLayer(m_doc->activeLayerId());
    refresh();
    emit compositeChanged();
    emit documentEdited(tr("Duplicate Layer"));
}

void LayersPanel::onMoveUp()
{
    if (!m_doc)
        return;
    const size_t idx = m_doc->layerIndex(m_doc->activeLayerId());
    if (idx == SIZE_MAX || idx + 1 >= m_doc->layers().size())
        return;
    m_doc->moveLayer(m_doc->activeLayerId(), idx + 1);
    refresh();
    emit compositeChanged();
    emit documentEdited(tr("Reorder Layers"));
}

void LayersPanel::onMoveDown()
{
    if (!m_doc)
        return;
    const size_t idx = m_doc->layerIndex(m_doc->activeLayerId());
    if (idx == SIZE_MAX || idx == 0)
        return;
    m_doc->moveLayer(m_doc->activeLayerId(), idx - 1);
    refresh();
    emit compositeChanged();
    emit documentEdited(tr("Reorder Layers"));
}

void LayersPanel::onMergeDown()
{
    if (!m_doc)
        return;
    if (m_doc->mergeDown(m_doc->activeLayerId())) {
        refresh();
        emit compositeChanged();
        emit documentEdited(tr("Merge Down"));
    }
}

} // namespace PhotoStudio::UI
