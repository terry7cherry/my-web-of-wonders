const getElementPosition = (({element, targetParent}) => {
    if (element.parent === targetParent) return {x: element.x, y: element.y}
    const parentPosition = getElementPosition({element: element.parent, targetParent});
    return {
        x: element.x + parentPosition.x,
        y: element.y + parentPosition.y
    }
})
export default ({
                    element,
                    targetParent
                }) => {
    const {x, y} = getElementPosition({element, targetParent});
    element.x = x;
    element.y = y;
    element.parent.removeChild(element);
    targetParent.addChild(element);
}