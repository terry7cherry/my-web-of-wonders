export default (ui, collector) => ({
    "topLeft": {
        x: ui.phaseContainer.x - 80,
        y: ui.phaseContainer.y - 80,
        controlX: 0,
        controlY: 0,
    },
    "topRight": {
        x: ui.phaseContainer.x + ui.phaseContainer.maxWidth - Math.max(0, (collector.sprite.width - 80)),
        y: ui.phaseContainer.y - 80,
        controlX: 160,
        controlY: 0,
    },
    "bottomLeft": {
        x: ui.phaseContainer.x - 80,
        y: ui.phaseContainer.y + ui.phaseContainer.maxHeight - Math.max(0, (collector.sprite.height - 80)),
        controlX: 0,
        controlY: 160,
    },
    "bottomRight": {
        x: ui.phaseContainer.x + ui.phaseContainer.maxWidth - Math.max(0, (collector.sprite.width - 80)),
        y: ui.phaseContainer.y + ui.phaseContainer.maxHeight - Math.max(0, (collector.sprite.height - 80)),
        controlX: 160,
        controlY: 160,
    }
});
