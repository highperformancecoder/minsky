abstract class TextInputUtilities {
    private static getContainer(): HTMLElement {
        return document.getElementsByClassName("direct-text-input-container")[0] as HTMLElement;
    }

    private static getInputElement(): HTMLInputElement {
        return document.getElementById("direct-text-input") as HTMLInputElement;
    }

    public static show() {
        this.getContainer().style.visibility = 'visible';
        this.getInputElement().focus();
    }

    public static hide() {
        this.setValue("");
        this.getContainer().style.visibility = 'hidden';
    }

    public static bindEvents() {
        this.getInputElement().addEventListener('keydown', async (event) => {
            event.stopPropagation();
        });
    }

    public static setValue(value: string) {
        this.getInputElement().value = value;
    }

    public static getValue(): string {
        return this.getInputElement().value;
    }
}

export { TextInputUtilities }