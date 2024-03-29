package home;

import java.util.ArrayList;
import java.util.List;

import org.apache.commons.lang3.RandomStringUtils;
import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttConnectOptions;
import org.eclipse.paho.client.mqttv3.MqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;
import org.eclipse.paho.client.mqttv3.MqttSecurityException;
import org.eclipse.paho.client.mqttv3.persist.MemoryPersistence;
import org.json.JSONObject;
import org.json.JSONTokener;

import com.pi4j.io.gpio.GpioController;
import com.pi4j.io.gpio.GpioFactory;
import com.pi4j.io.gpio.GpioPinDigitalInput;
import com.pi4j.io.gpio.GpioPinDigitalOutput;
import com.pi4j.io.gpio.PinPullResistance;
import com.pi4j.io.gpio.RaspiPin;
import com.pi4j.io.gpio.event.GpioPinDigitalStateChangeEvent;
import com.pi4j.io.gpio.event.GpioPinListenerDigital;

import home.model.Light2pi;

public class LightControlServer {

	final static GpioController gpio = GpioFactory.getInstance();
	final static GpioPinDigitalOutput GPIO_01 = gpio.provisionDigitalOutputPin(RaspiPin.GPIO_01);
	final static GpioPinDigitalInput pir = gpio.provisionDigitalInputPin(RaspiPin.GPIO_00, "PIR",
			PinPullResistance.PULL_DOWN);
	JSONObject json = new JSONObject();

	public static void main(String[] args) {

		// String topic = "home/";
		int qos = 0;
		String broker = "tcp://" + publicfield.MQTT_SERVER + ":" + publicfield.PORT;
		String clientId = RandomStringUtils.randomAlphanumeric(30).toUpperCase();

		final Boolean subscriber = true;
		final Boolean publisher = true;
		MemoryPersistence persistence = new MemoryPersistence();

		try {
			// Thread.sleep(10000);
			// GPIO_01.setState(true);
			MqttClient sampleClient = new MqttClient(broker, clientId, persistence);
			sampleClient.setCallback(new MqttCallback() { // 1
				@Override
				public void connectionLost(Throwable throwable) {
					// Called when connection is lost.

				}

				@Override
				public void messageArrived(String topic, MqttMessage mqttMessage) throws Exception {

					String str = new String(mqttMessage.getPayload());
					System.out.println("msg: " + str);
					if (Light2pi.GPIO_01.getValue().equals(topic)) {
						if (str.equals(home.model.Action.ON.getValue())) {
							GPIO_01.setState(true);
						}
						if (str.equals(home.model.Action.OFF.getValue())) {
							GPIO_01.setState(false);
						}

					}
					//home.light.state
					//if ("home.light.state.update".equals(topic)) {
					//	org.json.JSONTokener tokener = new JSONTokener(str);
					//	JSONObject lightJson = new JSONObject(jsonTokener);
					//	json.put(lightJson.get("id"), lightJson.get("value"))
					//}

					if (Light2pi.紅外線感應開關.getValue().equals(topic)) {
						if (str.equals("on")) {
							pir.addListener(new GpioPinListenerDigital() {
								public void handleGpioPinDigitalStateChangeEvent(GpioPinDigitalStateChangeEvent event) {
									// System.out.println(event.getState().getName());
									if (event.getState().isHigh()) {
										GPIO_01.setState(true);
									}

									if (event.getState().isLow()) {
										GPIO_01.setState(false);
									}

								}

							});
						} else {
							pir.removeAllListeners();
						}
					}

				}

				@Override
				public void deliveryComplete(final IMqttDeliveryToken iMqttDeliveryToken) {
					// When message delivery was complete
				}

			});
			sampleClient.connect();

			String[] t = { Light2pi.GPIO_01.getValue(), Light2pi.紅外線感應開關.getValue(), Light2pi.客廳電視插頭.getValue(),"home.light.state" };

			sampleClient.subscribe(t);
		} catch (Exception me) {
			// System.out.println("reason "+me.getReasonCode());
			System.out.println("msg " + me.getMessage());
			System.out.println("loc " + me.getLocalizedMessage());
			System.out.println("cause " + me.getCause());
			System.out.println("excep " + me);
			me.printStackTrace();
		}
	}

}
