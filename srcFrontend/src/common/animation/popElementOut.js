import getCoordinatesInParent from "./getCoordinatesInParent.js";

export default ({
                    element,
                    targetParent
                }) => {
    const {x, y} = getCoordinatesInParent({element, targetParent});
    element.x = x;
    element.y = y;
    element.parent.removeChild(element);
    targetParent.addChild(element);
}