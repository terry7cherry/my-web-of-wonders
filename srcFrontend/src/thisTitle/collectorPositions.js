export default (ui) => ({
    "topLeft": {
        x: ui.phaseContainer.x - ui.mainCollector.targetWidth,
        y: ui.phaseContainer.y - ui.mainCollector.targetHeight
    },
    "topRight": {
        x: ui.phaseContainer.x + ui.phaseContainer.maxWidth,
        y: ui.phaseContainer.y - ui.mainCollector.targetHeight
    },
    "bottomLeft": {
        x: ui.phaseContainer.x - ui.mainCollector.targetWidth,
        y: ui.phaseContainer.y + ui.phaseContainer.maxHeight
    },
    "bottomRight": {
        x: ui.phaseContainer.x + ui.phaseContainer.maxWidth,
        y: ui.phaseContainer.y + ui.phaseContainer.maxHeight
    }
});
