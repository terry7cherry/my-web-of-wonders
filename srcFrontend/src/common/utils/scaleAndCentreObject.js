export default (object, targetWidth, targetHeight) => {
    const scaleX = targetWidth / object.width;
    const scaleY = targetHeight / object.height;
    const scale = Math.min(scaleX, scaleY);
    object.scale.set(scale, scale);
    object.x = (targetWidth - object.width) / 2;
    object.y = (targetHeight - object.height) / 2;
}