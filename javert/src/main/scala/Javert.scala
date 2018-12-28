
import com.rabbitmq.client._
import org.json4s._
import org.json4s.native.JsonMethods._

package javerte {
	object Main {
		def untag(tagged: List[JField]): Map[String, JValue] = {
			(for {
				JField(name, JObject(tagval)) <- tagged
				JField("tag", JString(tag)) <- tagval
				JField("value", value) <- tagval
			} yield (name -> value)).toMap
		}

		def main(args: Array[String]): Unit = {
			val factory = new ConnectionFactory()
			val conn = factory.newConnection
			val channel = conn.createChannel

			channel.exchangeDeclare("jaffe", "direct", false)
			channel.queueBind("jiim", "jaffe", "")

			val deliverCallback: DeliverCallback = (consumerTag: String, delivery: Delivery) => {
				val encoded = new String(delivery.getBody, "UTF-8")
				val message = parse(encoded).asInstanceOf[JObject]

				val result = for {
					JObject(report) <- message
					JField("index", JInt(index)) <- report
					JField("location", JInt(location)) <- report
					JField("method", JString(method)) <- report
					JField("bindings", JObject(bindings)) <- report
				} yield (index, location, method, untag(bindings))

				println(result)
			}

			channel.basicConsume("jiim", true, deliverCallback, (consumerTag: String) => { println("Consuming...") });

			while(true) {}
//			channel.close
//			conn.close
		}
	}
}