export default (ui, collector) => ({
    "topLeft": {
        x: ui.phaseContainer.x - 80,
        y: ui.phaseContainer.y - 80
    },
    "topRight": {
        x: ui.phaseContainer.x + ui.phaseContainer.maxWidth - Math.max(0, (collector.sprite.width - 80)),
        y: ui.phaseContainer.y - 80
    },
    "bottomLeft": {
        x: ui.phaseContainer.x - 80,
        y: ui.phaseContainer.y + ui.phaseContainer.maxHeight - Math.max(0, (collector.sprite.height - 80))
    },
    "bottomRight": {
        x: ui.phaseContainer.x + ui.phaseContainer.maxWidth - Math.max(0, (collector.sprite.width - 80)),
        y: ui.phaseContainer.y + ui.phaseContainer.maxHeight - Math.max(0, (collector.sprite.height - 80))
    }
});
