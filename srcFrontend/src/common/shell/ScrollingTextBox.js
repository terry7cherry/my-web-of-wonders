export default class ScrollingTextBox {
    constructor(elementId) {
        this.textBox = document.getElementById(elementId);
        this.table = document.createElement("table");
        this.textBox.appendChild(this.table);
        Array(Math.ceil(this.textBox.clientHeight / 6))
            .fill("                    ")
            .forEach(text => this.addText(text));
        this.textBox.scrollTop = this.textBox.scrollHeight;
        this.autoScroll = true;

        this.textBox.addEventListener("scroll", () => {
            const tolerance = 50;
            const distanceFromBottom = this.textBox.scrollHeight - this.textBox.scrollTop - this.textBox.clientHeight;
            this.autoScroll = distanceFromBottom < tolerance;
        })
    }

    addText(text, onClick) {
        if (this.table.children.length >= 2000) this.table.removeChild(this.table.firstChild);

        const newRow = document.createElement("tr");

        if (!Array.isArray(text)) text = [text];
        newRow.innerHTML = text
            .map(c => `<td>${c.toString().replace(/\n/g, "")}</td>`)
            .join("");

        if (typeof onClick === "function") {
            newRow.style.cursor = "pointer";
            newRow.addEventListener("click", (e) => {
                onClick();
            });
        }

        this.table.appendChild(newRow);
        if (this.autoScroll) this.textBox.scrollTop = this.textBox.scrollHeight;
    }
}