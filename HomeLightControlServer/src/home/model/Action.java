package home.model;

public enum Action {
	ON("1"),OFF("0"),STATE("9");

	private final String value;

	private Action(String value) {
		this.value = value;
	}

	public String getValue() {
		return this.value;
	}

	public static Action ByValue(final String num) {

		for (Action e : Action.values()) {
			if (e.name().equals(num)) {
				return e;
			}
		}
		return null;
	}
}
