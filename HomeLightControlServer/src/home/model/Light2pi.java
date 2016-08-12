package home.model;

public enum Light2pi {
	GPIO_01("home.book.GPIO_01"),客廳電視插頭("home.liveroom.tv"),紅外線感應開關("home.book.irswitch");
	
	private final String value;

	private Light2pi(String value) {
		this.value = value;
	}

	public String getValue() {
		return this.value;
	}

	public static Light2pi ByValue(final String num) {

		for (Light2pi e : Light2pi.values()) {
			if (e.name() .equals(num)) {
				return e;
			}
		}
		return null;
	}
}
